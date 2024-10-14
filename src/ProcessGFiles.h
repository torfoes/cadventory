#ifndef PROCESS_G_FILES_H
#define PROCESS_G_FILES_H

#include <string>
#include <vector>
#include <filesystem>
#include <mutex>
#include <queue>
#include <thread>
#include <sqlite3.h>
#include <utility>
#include <map>

class ProcessGFiles {
public:
    ProcessGFiles();
    std::map<std::string, std::string> processGFile(const std::filesystem::path& file_path);
    std::map<std::string, std::string> executeMultiThreadedProcessing(const std::vector<std::string>& allGeometry, int num_workers = 4);

private:
    std::string dbPath;
    std::pair<std::string, std::string> runCommand(const std::string& command);
    void gFileWorker(std::queue<std::filesystem::path>& file_queue);
};

#endif // PROCESS_G_FILES_H
