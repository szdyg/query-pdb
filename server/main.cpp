#include <utility>
#include <set>
#include <algorithm>
#include <cstdlib>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>
#include "downloader.h"
#include "pdb_parser.h"




std::set<std::string> split(const std::string& s, char delimiter) {
    std::set<std::string> tokens;
    std::string token;
    std::istringstream token_stream(s);
    while (std::getline(token_stream, token, delimiter)) {
        tokens.insert(token);
    }
    return tokens;
}

// deliberately not using isalnum/isxdigit here, those depend on the locale and
// would accept bytes outside of ascii
bool is_ascii_alnum(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_ascii_hex(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

// the pdb name is used both as a path component below the save path and as part
// of the request line sent to the symbol server, so anything that could escape
// either of them has to be rejected
bool is_valid_pdb_name(const std::string &name) {
    if (name.empty() || name.size() > 128) {
        return false;
    }
    if (name.find("..") != std::string::npos) {
        return false;
    }
    return std::all_of(name.begin(), name.end(), [](char c) {
        return is_ascii_alnum(c) || c == '.' || c == '_' || c == '-' || c == '+';
    });
}

// 32 hex digits of the pe guid followed by the age, also in hex
bool is_valid_guid(const std::string &guid) {
    if (guid.size() < 33 || guid.size() > 40) {
        return false;
    }
    return std::all_of(guid.begin(), guid.end(), is_ascii_hex);
}

void set_error(httplib::Response &res, int status, const std::string &message) {
    res.status = status;
    res.set_header("Cache-Control", "no-store");
    res.set_content(message, "text/plain");
}

void set_result(httplib::Response &res, const std::string &body) {
    // a given (pdb, guid, query) always yields the same answer, so any cache in
    // front of us may keep the response indefinitely
    res.set_header("Cache-Control", "public, max-age=31536000, immutable");
    res.set_content(body, "application/json");
}

// fills in an error response and returns false when the request is not usable
bool parse_request(const httplib::Request &req, httplib::Response &res,
                   std::string &pdb, std::string &guid, std::set<std::string> &query) {
    if (!req.has_param("pdb") ||
        !req.has_param("guid") ||
        !req.has_param("query")) {
        set_error(res, 400, "missing pdb, guid or query parameter");
        return false;
    }

    pdb = req.get_param_value("pdb");
    if (!is_valid_pdb_name(pdb)) {
        set_error(res, 400, "invalid pdb name");
        return false;
    }

    guid = req.get_param_value("guid");
    if (!is_valid_guid(guid)) {
        set_error(res, 400, "invalid guid");
        return false;
    }

    query = split(req.get_param_value("query"), ',');
    if (query.empty()) {
        set_error(res, 400, "empty query");
        return false;
    }

    return true;
}

std::string require_env(const char *name) {
    const char *value = getenv(name);
    if (value == nullptr || *value == '\0') {
        spdlog::critical("environment variable {} is not set", name);
        std::exit(EXIT_FAILURE);
    }
    return value;
}

int main(int argc, char *argv[]) {

    std::string download_path = require_env("QUERY_PDB_SAVE_PATH");
    spdlog::info("pdb save path = {}", download_path);
    std::string download_server = require_env("MSDL_DOWNLOAD_SERVER");
    spdlog::info("pdb server = {}", download_server);

    downloader storage(download_path, download_server);

    httplib::Server server;
    server.set_exception_handler([](const auto &req, auto &res, std::exception_ptr ep) {
        std::string content;
        try {
            std::rethrow_exception(ep);
        } catch (std::exception &e) {
            content = e.what();
        } catch (...) {
            content = "Unknown Exception";
        }
        // a failure is almost always transient (the pdb was not downloadable yet),
        // caching it would keep serving the error long after it is fixed
        res.set_header("Cache-Control", "no-store");
        res.set_content(content, "plain/text");
        res.status = 500;

        spdlog::error("exception: {}", content);
    });

    server.Get("/symbol", [&storage](const httplib::Request &req, httplib::Response &res) {
        std::string pdb, guid;
        std::set<std::string> query_data;
        if (!parse_request(req, res, pdb, guid, query_data)) {
            return;
        }

        // download pdb
        if (!storage.download(pdb, guid)) {
            throw std::runtime_error("download failed");
        }

        // parse pdb
        pdb_parser parser(storage.get_path(pdb, guid).string());
        nlohmann::json result = parser.get_symbols(query_data);

        set_result(res, result.dump());
    });

    server.Get("/struct", [&storage](const httplib::Request &req, httplib::Response &res) {
        std::string pdb, guid;
        std::set<std::string> query_data;
        if (!parse_request(req, res, pdb, guid, query_data)) {
            return;
        }

        // download pdb
        if (!storage.download(pdb, guid)) {
            throw std::runtime_error("download failed");
        }

        // parse pdb
        pdb_parser parser(storage.get_path(pdb, guid).string());

        std::map<std::string, std::map<std::string, field_info> > result = parser.get_struct(query_data);

        nlohmann::json translate = nlohmann::json::object();
        for (const auto &[struct_name, fields]: result) {
            translate[struct_name] = nlohmann::json::object();
            for (const auto &[field_name, field]: fields) {
                translate[struct_name][field_name] = {
                        {"offset",          field.offset},
                        {"type",            field.type},
                        {"bitfield_offset", field.bitfield_offset},
                };
            }
        }

        set_result(res, translate.dump());
    });

    server.Get("/enum", [&storage](const httplib::Request &req, httplib::Response &res) {
        std::string pdb, guid;
        std::set<std::string> query_data;
        if (!parse_request(req, res, pdb, guid, query_data)) {
            return;
        }

        // download pdb
        if (!storage.download(pdb, guid)) {
            throw std::runtime_error("download failed");
        }

        // parse pdb
        pdb_parser parser(storage.get_path(pdb, guid).string());
        nlohmann::json result = parser.get_enum(query_data);

        set_result(res, result.dump());
    });

    server.listen("0.0.0.0", 8080);

    return 0;
}
