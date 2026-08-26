#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
// CPPHTTPLIB_OPENSSL_SUPPORT is provided by the httplib cmake target, defining it
// here again only produces a -Wmacro-redefined warning
#include <httplib.h>
#include <spdlog/spdlog.h>
#include "downloader.h"

downloader::downloader(std::string save_path, std::string msdl_server)
    : save_path_(std::move(save_path)),
      msdl_server_(std::move(msdl_server)) {
    if (msdl_server_.empty() || save_path_.empty()) {
        throw std::invalid_argument("downloader requires a non-empty save path and server");
    }

    if (msdl_server_.back() == '/') {
        msdl_server_.pop_back();
    }
    spdlog::info("save path: {}, server: {}", save_path_, msdl_server_);
}

bool downloader::download(const std::string &name, const std::string &guid) {
    std::string relative_path = get_relative_path_str(name, guid);
    auto path = get_path(name, guid);
    spdlog::trace("lookup pdb, path: {}", relative_path);

    // a completed download is published with an atomic rename, so a file that
    // exists is always a whole file and can be used without taking any lock
    if (std::filesystem::exists(path)) {
        spdlog::trace("pdb already exists, path: {}", relative_path);
        return true;
    }

    auto lock = file_lock(relative_path);
    std::lock_guard<std::mutex> guard(*lock);

    // another request may have downloaded it while we waited for the lock
    if (std::filesystem::exists(path)) {
        spdlog::trace("pdb downloaded by another request, path: {}", relative_path);
        return true;
    }

    return download_impl(relative_path);
}

std::shared_ptr<std::mutex> downloader::file_lock(const std::string &relative_path) {
    std::lock_guard<std::mutex> guard(locks_mutex_);

    // reclaim the locks nobody holds anymore before inserting a new one
    for (auto it = locks_.begin(); it != locks_.end();) {
        it = it->second.expired() ? locks_.erase(it) : std::next(it);
    }

    auto &slot = locks_[relative_path];
    auto lock = slot.lock();
    if (!lock) {
        lock = std::make_shared<std::mutex>();
        slot = lock;
    }
    return lock;
}

std::string downloader::get_relative_path_str(const std::string &name, const std::string &guid) {
    return name + '/' + guid + '/' + name;
}

std::filesystem::path downloader::get_path(const std::string &name, const std::string &guid) {
    std::string relative_path = get_relative_path_str(name, guid);
    auto path = std::filesystem::path(save_path_).append(relative_path);
    return path;
}

bool downloader::download_impl(const std::string &relative_path) {
    spdlog::info("download pdb, path: {}", relative_path);
    auto path = std::filesystem::path(save_path_).append(relative_path);
    std::filesystem::create_directories(path.parent_path());
    httplib::Client client(msdl_server_);
    client.set_follow_location(true);
    std::string res_url = "/download/symbols/" + relative_path;
    auto res = client.Get(res_url);
    if (res.error() != httplib::Error::Success) {
        spdlog::error("failed to download pdb, path: {} error: {}", res_url,to_string(res.error()));
        return false;
    }
    if (res->status != 200) {
        spdlog::error("failed to download pdb, path: {} http: {}", res_url,res->status);
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
