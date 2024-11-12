#include "ProcessGFiles.h"

#include <QFileInfo>
#include <QProcess>
#include <QSettings>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <regex>
#include <set>
#include <sstream>
#include <thread>
#include <vector>

#include "brlcad/ged.h"
#include "brlcad/ged.h"
#include "brlcad/bu.h"
#include "brlcad/rt/db_attr.h"


#include "config.h"

namespace fs = std::filesystem;

ProcessGFiles::ProcessGFiles(Model* model, bool debug)
    : model(model), debug(debug) {}

void ProcessGFiles::debugPrint(const std::string& message) {
  if (debug) {
    std::cout << message << std::endl;
  }
}

void ProcessGFiles::debugError(const std::string& message) {
  if (debug) {
    std::cerr << message << std::endl;
  }
}
void ProcessGFiles::extractTitle(ModelData& modelData, struct ged* gedp) {
    const char* title = db_get_title(gedp->dbip);
    if (title && title[0] != '\0') {
        modelData.title = title;
    } else {
        modelData.title = "Unknown";
    }
}

void ProcessGFiles::extractObjects(ModelData& modelData, struct ged* gedp) {
    this->objects.clear();
    this->parentRelations.clear();
    this->current_model_id = modelData.id;

    // Get the list of all objects in the database
    struct directory** dir;
    int dir_count;
    int flags = 0;  // No specific flags

    if (db_ls(gedp->dbip, flags, NULL, &dir, &dir_count) < 0) {
        bu_log("Failed to list objects in the database.\n");
        return;
    }

    std::set<std::string> object_names;

    for (int i = 0; i < dir_count; ++i) {
        struct directory* dp = dir[i];
        const char* object_name = dp->d_namep;
        bool is_combination = dp->d_flags & RT_DIR_COMB;

        ObjectData objectData;
        objectData.model_id = current_model_id;
        objectData.name = object_name;
        objectData.parent_object_id = -1;  // We'll set this later
        objectData.is_selected = false;    // You can set your selection logic here

        this->objects.push_back(objectData);
        object_names.insert(object_name);

        // If the object is a combination, get its members
        if (is_combination) {
            struct rt_db_internal intern;
            if (rt_db_get_internal(&intern, dp, gedp->dbip, NULL, &rt_uniresource) < 0) {
                bu_log("Failed to get internal form of %s\n", object_name);
                continue;
            }

            struct rt_comb_internal* comb = (struct rt_comb_internal*)intern.idb_ptr;
            if (comb->tree) {
                std::set<std::string> child_objects;
                // Recursively extract child object names
                db_tree_funcleaf(gedp->dbip, comb->tree, [](const struct db_i*, const union tree* tp, void* client_data) {
                    std::set<std::string>* child_objects = static_cast<std::set<std::string>*>(client_data);
                    if (tp->tr_op == OP_DB_LEAF && tp->tr_l.tl_name) {
                        child_objects->insert(tp->tr_l.tl_name);
                    }
                    return 0;
                }, &child_objects);

                // Update parent relations
                for (const auto& child_name : child_objects) {
                    parentRelations[child_name] = object_name;
                }
            }

            rt_db_free_internal(&intern);
        }
    }

    bu_free(dir, "free directory array");
}

void ProcessGFiles::processGFile(const fs::path& file_path,
                                 const std::string& previews_folder,
                                 const std::string& library_name) {
    try {
        // Open the database using libged
        struct ged* gedp = ged_open("db", file_path.string().c_str(), 1);
        if (gedp == GED_NULL) {
            bu_log("ERROR: Unable to open database file %s\n", file_path.string().c_str());
            return;
        }

        QSettings settings;
        bool previewFlag = settings.value("previewFlag", true).toBool();

        std::string model_short_name = file_path.stem().string();

        // Check if the model already exists
        ModelData existingModel = model->getModelByFilePath(file_path.string());

        if (existingModel.id != 0 && existingModel.is_processed) {
            std::cout << "Model already processed: " << model_short_name << "\n";
            ged_close(gedp);
            return;
        }

        // Prepare ModelData
        ModelData modelData;
        modelData.short_name = model_short_name;
        modelData.primary_file = file_path.filename().string();
        modelData.file_path = file_path.string();
        modelData.library_name = library_name;
        modelData.is_selected = false;
        modelData.is_included = true;

        // Extract title using gedp
        extractTitle(modelData, gedp);

        // Extract objects using gedp
        extractObjects(modelData, gedp);

        // Determine selected object name
        std::string selected_object_name;
        if (!objects.empty()) {
            selected_object_name = objects.front().name;  // Simple selection logic
            for (auto& obj : objects) {
                obj.is_selected = (obj.name == selected_object_name);
            }
        }

        // Generate thumbnail
        if (previewFlag) {
            generateThumbnail(modelData, file_path.string(), previews_folder, selected_object_name);
        }

        modelData.is_processed = true;

        // Insert or update the model in the database
        if (existingModel.id != 0) {
            // Model exists, update it
            model->updateModel(existingModel.id, modelData);
            modelData.id = existingModel.id;
        } else {
            // Model does not exist, insert it
            bool insertSuccess = model->insertModel(modelData);
            if (!insertSuccess) {
                std::cerr << "Failed to insert model: " << model_short_name << "\n";
                ged_close(gedp);
                return;
            }
            // Retrieve the inserted model to get the assigned id
            ModelData insertedModel = model->getModelByFilePath(file_path.string());
            if (insertedModel.id == 0) {
                std::cerr << "Failed to retrieve inserted model: " << model_short_name << "\n";
                ged_close(gedp);
                return;
            }
            modelData.id = insertedModel.id;
        }

        // Delete existing objects if any
        model->deleteObjectsForModel(modelData.id);

        // Map from object name to object_id
        std::map<std::string, int> objectNameToId;

        // Begin transaction
        model->beginTransaction();

        // Insert objects and collect mapping
        for (auto& objData : objects) {
            objData.model_id = modelData.id;
            int objectId = model->insertObject(objData);
            objectNameToId[objData.name] = objectId;
        }

        // Update parent_object_id for each object
        for (auto& objData : objects) {
            auto it = parentRelations.find(objData.name);
            if (it != parentRelations.end()) {
                std::string parentName = it->second;
                int objectId = objectNameToId[objData.name];
                int parentObjectId = objectNameToId[parentName];
                model->updateObjectParentId(objectId, parentObjectId);
            }
        }

        model->commitTransaction();

        ged_close(gedp);

    } catch (const std::exception& e) {
        debugError("Error processing file " + file_path.string() + ": " + e.what());
    }
}


// void ProcessGFiles::generateThumbnail(ModelData& modelData,
//                                       const std::string& file_path,
//                                       const std::string& previews_folder,
//                                       const std::string& selected_object_name) {
//     std::string rt_executable = RT_EXECUTABLE_PATH;
//     std::string model_short_name = fs::path(file_path).stem().string();

//     QSettings settings;
//     int timeLimit = settings.value("previewTimer", 30).toInt();

//     if (selected_object_name.empty()) {
//         std::cerr << "No valid object selected for raytrace in file: " << file_path
//                   << "\n";
//         return;
//     }

//     // Generate thumbnail
//     std::string png_file = previews_folder + "/" + model_short_name + ".png";
//     fs::create_directories(fs::path(png_file).parent_path());
//     std::string rt_command = rt_executable + " -s512 -o \"" + png_file + "\" \"" +
//                              file_path + "\" " + selected_object_name;

//     try {
//         auto [rt_output, rt_error, rt_return_code] =
//             runCommand(rt_command, timeLimit);
//         std::this_thread::sleep_for(std::chrono::milliseconds(100));

//         if (fs::exists(png_file) && fs::file_size(png_file) > 0) {
//             // Read thumbnail data
//             std::ifstream thumbnail_file(png_file, std::ios::binary);
//             std::vector<char> thumbnail_data(
//                 (std::istreambuf_iterator<char>(thumbnail_file)),
//                 std::istreambuf_iterator<char>());
//             thumbnail_file.close();

//             modelData.thumbnail = thumbnail_data;

//             // Optionally delete the PNG file
//             fs::remove(png_file);
//         } else {
//             std::cerr << "Raytrace failed for object: " << selected_object_name
//                       << "\n";
//         }
//     } catch (const std::exception& e) {
//         std::cerr << "Raytrace failed for '" << selected_object_name
//                   << "': " << e.what() << "\n";
//     }
// }

