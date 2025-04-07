#include <utility>
#include <set>
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

int main(int argc, char *argv[]) {

    std::string download_path  = getenv("QUERY_PDB_SAVE_PATH");
    spdlog::info("pdb save path = {}", download_path);
    std::string download_server  = getenv("MSDL_DOWNLOAD_SERVER");
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
        res.set_content(content, "plain/text");
        res.status = 500;

        spdlog::error("exception: {}", content);
    });

    server.Get("/symbol", [&storage](const httplib::Request &req, httplib::Response &res) {
        if (!req.has_param("pdb") ||
            !req.has_param("guid") ||
            !req.has_param("query") ) {
            res.status = 400;
            res.set_content("invaild params", "application/json");
            return;
        }
        auto query = req.get_param_value("query");
        auto pdb = req.get_param_value("pdb");
        auto guid = req.get_param_value("guid");
        auto query_data = split(query, ',');

        // download pdb
        if (!storage.download(pdb, guid)) {
            throw std::runtime_error("download failed");
        }

        // parse pdb
        pdb_parser parser(storage.get_path(pdb, guid).string());
        nlohmann::json result = parser.get_symbols(query_data);

        res.set_content(result.dump(), "application/json");
    });

    server.Get("/struct", [&storage](const httplib::Request &req, httplib::Response &res) {
        if (!req.has_param("pdb") ||
            !req.has_param("guid") ||
            !req.has_param("query")){
            res.status = 400;
            res.set_content("invaild params", "application/json");
            return;
        }

        auto query = req.get_param_value("query");
        auto pdb = req.get_param_value("pdb");
        auto guid = req.get_param_value("guid");
        auto query_data=  split(query, ',');

        // download pdb
        if (!storage.download(pdb, guid)) {
            throw std::runtime_error("download failed");
        }

        // parse pdb
        pdb_parser parser(storage.get_path(pdb, guid).string());

        std::map<std::string, std::map<std::string, field_info> > result =parser.get_struct(query_data);

        std::map<std::string, std::map<std::string, std::map<std::string, int64_t> > > translate;
        for (const auto &[struct_name, fields]: result) {
            translate[struct_name] = {};
            for (const auto &[field_name, field]: fields) {
                translate[struct_name][field_name] = field.to_map();
            }
        }

        res.set_content(nlohmann::json(translate).dump(), "application/json");
    });

    server.Get("/enum", [&storage](const httplib::Request &req, httplib::Response &res) {

        if (!req.has_param("pdb") ||
            !req.has_param("guid") ||
            !req.has_param("query")){
            res.status = 400;
            res.set_content("invaild params", "application/json");
            return;
        }

        auto query = req.get_param_value("query");
        auto pdb = req.get_param_value("pdb");
        auto guid = req.get_param_value("guid");
        auto query_data=  split(query, ',');

        // download pdb
        if (!storage.download(pdb, guid)) {
            throw std::runtime_error("download failed");
        }

        // parse pdb
        pdb_parser parser(storage.get_path(pdb, guid).string());
        nlohmann::json result = parser.get_enum(query_data);

        res.set_content(result.dump(), "application/json");
    });

    server.listen("0.0.0.0", 8080);

    return 0;
}
