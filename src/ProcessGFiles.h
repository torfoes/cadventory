#ifndef PROCESSGFILES_H
#define PROCESSGFILES_H

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <filesystem>

#include "Model.h"

class ProcessGFiles {
public:
    explicit ProcessGFiles(Model* model, bool debug = false);

    void processGFile(const std::filesystem::path& file_path, const std::string& previews_folder, const std::string& library_name);

private:
    // Helper methods
    void debugPrint(const std::string& message);
    void debugError(const std::string& message);
    bool isModelProcessed(int modelId);
    void extractTitle(ModelData& modelData, const std::string& file_path);
    void extractObjects(ModelData& modelData, const std::string& file_path);
    void generateThumbnail(ModelData& modelData, const std::string& file_path, const std::string& previews_folder);

    // Command execution helpers
    std::tuple<std::string, std::string, int> runCommand(const std::string& command, int timeout_seconds = 10);
    std::vector<std::string> parseTopsOutput(const std::string& tops_output);
    std::vector<std::string> parseLtOutput(const std::string& lt_output);
    bool validateObject(const std::string& file_path, const std::string& object_name);

    Model* model;
    bool debug;
    std::mutex db_mutex;
};

#endif // PROCESSGFILES_H
