#ifndef QUERY_PDB_SERVER_DOWNLOADER_H
#define QUERY_PDB_SERVER_DOWNLOADER_H

#include <string>
#include <mutex>
#include <shared_mutex>
#include <filesystem>

class downloader {
public:
    downloader(std::string save_path, std::string msdl_server);

    bool download(const std::string &name, const std::string &guid);

    std::filesystem::path get_path(const std::string &name, const std::string &guid);

private:
    std::string save_path_;
    std::string msdl_server_;
    std::shared_mutex mutex_;

    static std::string get_relative_path_str(const std::string &name, const std::string &guid);
    bool download_impl(const std::string &relative_path);
    bool download_cache(const std::string &path);
};

#endif //QUERY_PDB_SERVER_DOWNLOADER_H
