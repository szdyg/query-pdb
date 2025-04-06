#include <sstream>
#include <filesystem>
#include <fstream>
#include <regex>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <spdlog/spdlog.h>
#include "downloader.h"

downloader::downloader(std::string save_path, std::string msdl_server)
    : save_path_(std::move(save_path)),
      msdl_server_(std::move(msdl_server)) {
    spdlog::info("create downloader, path: {}, server: {}", save_path_, msdl_server_);
    if (msdl_server_.empty() || save_path_.empty()) {
        spdlog::error("invalid downloader, path: {}, server: {}", save_path_, msdl_server_);
        return;
    }
    if (msdl_server_.back() != '/') {
        msdl_server_.push_back('/');
    }

    server_split_ = split_server_name();
    if (server_split_.first.empty()) {
        spdlog::error("split server name failed, server: {}", msdl_server_);
        return;
    }
}

bool downloader::download(const std::string &name, const std::string &guid, uint32_t age) {
    std::string relative_path = get_relative_path_str(name, guid, age);
    auto path = get_path(name, guid, age);
    spdlog::trace("lookup pdb, path: {}", relative_path);

    if (download_cache(path.string())) {
        spdlog::trace("pdb already exists, path: {}", relative_path);
        return true;
    }

    return download_impl(relative_path);
}

std::string downloader::get_relative_path_str(const std::string &name, const std::string &guid, uint32_t age) {
    std::stringstream ss;
    ss << std::hex << std::uppercase;
    ss << name << '/' << guid << age << '/' << name;
    return ss.str();
}

std::filesystem::path downloader::get_path(const std::string &name, const std::string &guid, uint32_t age) {
    std::string relative_path = get_relative_path_str(name, guid, age);
    auto path = std::filesystem::path(save_path_).append(relative_path);
    return path;
}

bool downloader::download_impl(const std::string &relative_path) {
    spdlog::info("download pdb, path: {}", relative_path);

    std::unique_lock<std::shared_mutex> lck(mutex_);
    auto path = std::filesystem::path(save_path_).append(relative_path);
    std::filesystem::create_directories(path.parent_path());
    std::string buf;
    httplib::Client client(server_split_.first);
    client.set_follow_location(true);
    auto res = client.Get(server_split_.second + relative_path);
    if (res.error() != httplib::Error::Success ||
        !res ||
        res->status != 200) {
        spdlog::error("failed to download pdb, path: {}", relative_path);
        return false;
    }

    // download file size check
    size_t content_length = 0;
    if (res->has_header("Content-Length")) {
        content_length = std::stoul(res->get_header_value("Content-Length"));
    }
    if (content_length == 0 || content_length != res->body.size()) {
        spdlog::error("downloaded pdb size mismatch, path: {}", relative_path);
        return false;
    }

    auto tmp_path = path;
    tmp_path.replace_extension(".tmp");
    std::ofstream f(tmp_path, std::ios::binary);
    if (!f.is_open()) {
        spdlog::error("failed to open file, path: {}", tmp_path.string());
        return false;
    }
    f.write(res->body.c_str(), static_cast<std::streamsize>(res->body.size()));
    f.close();

    std::filesystem::rename(tmp_path, path);
    spdlog::info("download pdb success, path: {}", relative_path);
    return true;
}

bool downloader::download_cache(const std::string &path) {
    std::shared_lock<std::shared_mutex> lck(mutex_);
    if (std::filesystem::exists(path)) {
        spdlog::trace("pdb already exists, path: {}", path);
        return true;
    }
    return false;
}

std::pair<std::string, std::string> downloader::split_server_name() {
    std::regex regex(R"(^((?:(?:http|https):\/\/)?[^\/]+)(\/.*)$)");
    std::smatch match;

    if (!std::regex_match(msdl_server_, match, regex)) {
        return {};
    }

    return {match[1].str(), match[2].str()};
}
