#ifndef PROCESSGFILES_H
#define PROCESSGFILES_H

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <filesystem>
#include <map> // Include map for parentRelations

#include "Model.h"

class ProcessGFiles {
public:
    explicit ProcessGFiles(Model* model);

    void processGFile(const std::filesystem::path& file_path, const std::string& previews_folder, bool preview);
    std::tuple<bool, std::string> generateGistReport(const std::string& inputFilePath, const std::string& outputFilePath, const std::string& primary_obj);


private:
    // Helper methods
    void extractTitle(ModelData& modelData, const std::string& file_path);

    std::vector<ObjectData> extractObjects(
        const ModelData& modelData,
        const std::string& file_path,
        std::map<std::string, std::string>& parentRelations);

    void generateThumbnail(
        ModelData& modelData,
        const std::string& file_path,
        const std::string& previews_folder,
        const std::string& selected_object_name);

    // Command execution helpers
    std::tuple<std::string, std::string, int> runCommand(const std::string& command, int timeout_seconds = 10);
    std::vector<std::string> parseTopsOutput(const std::string& tops_output);
    std::vector<std::string> parseLtOutput(const std::string& lt_output);
    bool validateObject(const std::string& file_path, const std::string& object_name);
    std::vector<std::string> splitStringByWhitespace(const std::string &input);

    Model* model;
    std::mutex db_mutex;
};

#endif // PROCESSGFILES_H
