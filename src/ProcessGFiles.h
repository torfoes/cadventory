#ifndef PROCESSGFILES_H
#define PROCESSGFILES_H

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "Model.h"

class ProcessGFiles {
public:
    explicit ProcessGFiles(Model* model, bool debug = false);

    void processGFile(const std::filesystem::path& file_path,
                      const std::string& previews_folder,
                      const std::string& library_name = "(unknown)");

private:
    void extractTitle(ModelData& modelData, struct ged* gedp);

    // void extractObjects(ModelData& modelData, struct ged* gedp);

    // void generateThumbnail(ModelData& modelData, const std::string& file_path,
    //                        const std::string& previews_folder,
    //                        const std::string& selected_object_name);

    // Member variables
    Model* model;

    // Member variables for traversal
    // std::vector<ObjectData> objects;
    // std::map<std::string, std::string> parentRelations;
    // int current_model_id;
    // std::string selected_object_name;
};

#endif  // PROCESSGFILES_H
