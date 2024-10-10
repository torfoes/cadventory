#ifndef PROCESS_G_FILES_H
#define PROCESS_G_FILES_H

#include <string>
#include <vector>
#include <filesystem>
#include <mutex>
#include <sqlite3.h>
#include <utility>

class ProcessGFiles {
public:
    ProcessGFiles(const std::string& dbPath);
    void processGFile(const std::filesystem::path& file_path);
    void executeBackgroundProcess(const std::filesystem::path& directory, int num_workers = 4);

private:
    std::string dbPath;
    void createDatabase();
    std::pair<std::string, std::string> runCommand(const std::string& command);
};

#endif