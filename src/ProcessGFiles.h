// ProcessGFiles.h

#ifndef PROCESSGFILES_H
#define PROCESSGFILES_H

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <filesystem>

class Model;

class ProcessGFiles {
public:
    explicit ProcessGFiles(Model* model);

    void executeMultiThreadedProcessing(const std::vector<std::string>& allGeometry, int num_workers = 4);
    void processGFile(const std::filesystem::path& file_path, const std::string& previews_folder);

private:
    void gFileWorker(std::queue<std::filesystem::path>& file_queue, const std::string& previews_folder);
    std::tuple<std::string, std::string, int> runCommand(const std::string& command, int timeout_seconds = 10);
    std::vector<std::string> parseTopsOutput(const std::string& tops_output);
    bool validateObject(const std::string& file_path, const std::string& object_name);

    Model* model;
    std::mutex queue_mutex;
};

#endif // PROCESSGFILES_H
