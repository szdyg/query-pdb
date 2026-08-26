#ifndef QUERY_PDB_SERVER_DOWNLOADER_H
#define QUERY_PDB_SERVER_DOWNLOADER_H

#include <string>
#include <map>
#include <memory>
#include <mutex>
#include <filesystem>

class downloader {
public:
    downloader(std::string save_path, std::string msdl_server);

    bool download(const std::string &name, const std::string &guid);

    std::filesystem::path get_path(const std::string &name, const std::string &guid);

private:
    std::string save_path_;
    std::string msdl_server_;

    // one lock per pdb, so that downloading a cold pdb never blocks requests for
    // an unrelated one. entries are handed out as shared_ptr and reclaimed once
    // the last downloader of that pdb is done, so the map stays as small as the
    // number of downloads currently in flight.
    std::mutex locks_mutex_;
    std::map<std::string, std::weak_ptr<std::mutex>> locks_;

    static std::string get_relative_path_str(const std::string &name, const std::string &guid);
    std::shared_ptr<std::mutex> file_lock(const std::string &relative_path);
    bool download_impl(const std::string &relative_path);
};

#endif //QUERY_PDB_SERVER_DOWNLOADER_H
