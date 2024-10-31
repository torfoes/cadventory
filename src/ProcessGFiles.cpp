#include "ProcessGFiles.h"
#include "config.h"

#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <mutex>
#include <cstdlib>
#include <cstdio>
#include <array>
#include <sstream>
#include <chrono>
#include <filesystem>
#include <algorithm>
#include <map>
#include <queue>
#include <set>
#include <regex>

namespace fs = std::filesystem;

ProcessGFiles::ProcessGFiles(Model* model) : model(model) {
}

bool ProcessGFiles::isModelProcessed(int modelId) {
    ModelData existingModel = model->getModelById(modelId);
    // Check if the model exists and has a thumbnail (or any other criteria)
    return existingModel.id == modelId && !existingModel.thumbnail.empty();
}

void ProcessGFiles::extractTitle(ModelData& modelData, const std::string& file_path) {
    std::string mged_executable = MGED_EXECUTABLE_PATH;
    std::string title_command = mged_executable + " -c \"" + file_path + "\" title";
    auto [title_output, title_error, title_return_code] = runCommand(title_command);
    std::string title = !title_output.empty() ? title_output : title_error;
    title = title.empty() ? "Unknown" : title;
    title.erase(title.find_last_not_of(" \n\r\t") + 1);

    std::cout << "Title extracted: " << title << "\n";

    modelData.title = title;
}

void ProcessGFiles::extractObjects(ModelData& modelData, const std::string& file_path) {
    std::string mged_executable = MGED_EXECUTABLE_PATH;

    // Set the maximum depth for traversal
    int max_depth = 3;
    
    // Use BFS to traverse the object hierarchy
    // Queue now includes the current depth
    std::queue<std::tuple<std::string, int, int>> object_queue; // (object_name, parent_object_id, depth)
    std::set<std::string> processed_objects;

    // Get top-level objects
    std::string tops_command = mged_executable + " -c \"" + file_path + "\" tops -n";
    std::cout << "Executing tops command: " << tops_command << std::endl;
    auto [tops_output, tops_error, tops_return_code] = runCommand(tops_command);
    std::string tops_result = !tops_output.empty() ? tops_output : tops_error;
    std::cout << "tops command output:\n" << tops_result << std::endl;
    std::cout << "tops command return code: " << tops_return_code << std::endl;

    // Parse the tops output
    std::vector<std::string> tops_elements = parseTopsOutput(tops_result);
    std::cout << "Parsed top-level elements: ";
    for (const auto& elem : tops_elements) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;

    // Insert top-level objects
    for (const auto& top_element : tops_elements) {
        if (processed_objects.find(top_element) != processed_objects.end()) {
            std::cout << "Already processed top element: " << top_element << std::endl;
            continue;
        }

        std::cout << "Inserting top-level object: " << top_element << std::endl;

        ObjectData objectData;
        objectData.model_id = modelData.id;
        objectData.name = top_element;
        objectData.parent_object_id = -1; // No parent for top-level objects
        objectData.is_selected = true;    // Default selection state

        int objectId = model->insertObject(objectData);
        if (objectId != -1) {
            std::cout << "Inserted object '" << top_element << "' with ID " << objectId << std::endl;
            int initial_depth = 1; // Depth of top-level objects is 1
            object_queue.emplace(top_element, objectId, initial_depth);
            processed_objects.insert(top_element);
        } else {
            std::cerr << "Failed to insert object: " << top_element << std::endl;
        }
    }

    // Process the queue with depth limit
    while (!object_queue.empty()) {
        auto [current_object, parent_object_id, current_depth] = object_queue.front();
        object_queue.pop();

        std::cout << "Processing object: " << current_object
                  << ", parent ID: " << parent_object_id
                  << ", depth: " << current_depth << std::endl;

        // Check if we've reached the maximum depth
        if (current_depth >= max_depth) {
            std::cout << "Reached maximum depth for object: " << current_object << std::endl;
            continue;
        }

        // Get child objects
        std::string lt_command = mged_executable + " -c \"" + file_path + "\" lt " + current_object;
        std::cout << "Executing lt command: " << lt_command << std::endl;
        auto [lt_output, lt_error, lt_return_code] = runCommand(lt_command);
        std::string lt_result = !lt_output.empty() ? lt_output : lt_error;
        std::cout << "lt command output:\n" << lt_result << std::endl;
        std::cout << "lt command return code: " << lt_return_code << std::endl;

        // Parse the lt output
        std::vector<std::string> child_objects = parseLtOutput(lt_result);
        std::cout << "Parsed child objects of " << current_object << ": ";
        for (const auto& child : child_objects) {
            std::cout << child << " ";
        }
        std::cout << std::endl;

        for (const auto& child_object : child_objects) {
            if (processed_objects.find(child_object) != processed_objects.end()) {
                std::cout << "Already processed child object: " << child_object << std::endl;
                continue;
            }

            std::cout << "Inserting child object: " << child_object
                      << ", parent ID: " << parent_object_id << std::endl;

            ObjectData objectData;
            objectData.model_id = modelData.id;
            objectData.name = child_object;
            objectData.parent_object_id = parent_object_id;
            objectData.is_selected = true; // Default selection state

            int objectId = model->insertObject(objectData);


            if (objectId != -1) {
                std::cout << "Inserted object '" << child_object << "' with ID " << objectId << std::endl;
                int child_depth = current_depth + 1;
                object_queue.emplace(child_object, objectId, child_depth);
                processed_objects.insert(child_object);
            } else {
                std::cerr << "Failed to insert object: " << child_object << std::endl;
            }
        }
    }
}


void ProcessGFiles::generateThumbnail(ModelData& modelData, const std::string& file_path, const std::string& previews_folder) {
    std::string rt_executable = RT_EXECUTABLE_PATH;
    std::string model_short_name = fs::path(file_path).stem().string();

    // Determine objects to try for raytracing
    std::vector<std::string> objects_to_try = {"all", "all.g", model_short_name, model_short_name + ".g", model_short_name + ".c"};

    // Validate objects
    std::vector<std::string> valid_objects;
    for (const auto& obj : objects_to_try) {
        if (validateObject(file_path, obj)) {
            valid_objects.push_back(obj);
        }
    }

    if (valid_objects.empty()) {
        std::cerr << "No valid objects to raytrace in file: " << file_path << "\n";
        return;
    }

    // Generate thumbnail
    std::string png_file = previews_folder + "/" + model_short_name + ".png";
    fs::create_directories(fs::path(png_file).parent_path());
    std::string rt_command_template = rt_executable + " -s512 -o \"" + png_file + "\" \"" + file_path + "\" ";

    bool raytrace_successful = false;
    for (const auto& obj : valid_objects) {
        std::string rt_command = rt_command_template + obj;
        try {
            auto [rt_output, rt_error, rt_return_code] = runCommand(rt_command, 30);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            if (fs::exists(png_file) && fs::file_size(png_file) > 0) {
                raytrace_successful = true;

                // Read thumbnail data
                std::ifstream thumbnail_file(png_file, std::ios::binary);
                std::vector<char> thumbnail_data((std::istreambuf_iterator<char>(thumbnail_file)), std::istreambuf_iterator<char>());
                thumbnail_file.close();

                modelData.thumbnail = thumbnail_data;

                // Optionally delete the PNG file
                fs::remove(png_file);
                break;
            }
        } catch (const std::exception& e) {
            std::cerr << "Raytrace failed for '" << obj << "': " << e.what() << "\n";
        }
    }

    if (!raytrace_successful) {
        std::cerr << "Raytrace failed for all possible objects in " << model_short_name << ".\n";
    }
}

void ProcessGFiles::processGFile(const fs::path& file_path, const std::string& previews_folder) {
    try {
        std::string model_short_name = file_path.stem().string();
        int modelId = model->hashModel(file_path.string());

        if (isModelProcessed(modelId)) {
            std::cout << "Model already processed: " << model_short_name << "\n";
            return;
        }

        // Prepare ModelData
        ModelData modelData;
        modelData.id = modelId;
        modelData.short_name = model_short_name;
        modelData.primary_file = file_path.filename().string();
        modelData.file_path = file_path.string();
        modelData.library_name = "non library";
        modelData.isSelected = false;

        // Insert the model
        model->insertModel(modelId, modelData);

        // Extract title
        extractTitle(modelData, file_path.string());

        // Delete existing objects if any
        model->deleteObjectsForModel(modelId);

        // Extract objects
        extractObjects(modelData, file_path.string());

        // Generate thumbnail
        generateThumbnail(modelData, file_path.string(), previews_folder);

        // Update model
        model->updateModel(modelId, modelData);

    } catch (const std::exception& e) {
        std::cerr << "Error processing file " << file_path << ": " << e.what() << "\n";
    }
}

std::tuple<std::string, std::string, int> ProcessGFiles::runCommand(const std::string& command, int timeout_seconds) {
    std::array<char, 256> buffer;
    std::string output;

    FILE* pipe = popen((command + " 2>&1").c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    auto start_time = std::chrono::steady_clock::now();

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        output += buffer.data();

        auto elapsed = std::chrono::steady_clock::now() - start_time;
        if (std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() > timeout_seconds) {
            pclose(pipe);
            throw std::runtime_error("Command timed out");
        }
    }

    int return_code = pclose(pipe);

    return {output, "", return_code};
}

std::vector<std::string> ProcessGFiles::parseTopsOutput(const std::string& tops_output) {
    std::vector<std::string> tops_elements;
    std::istringstream iss(tops_output);
    for (std::string line; std::getline(iss, line); ) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (!line.empty()) {
            tops_elements.push_back(line);
            std::cout << "Found top-level object: " << line << std::endl;
        }
    }
    return tops_elements;
}

std::vector<std::string> ProcessGFiles::parseLtOutput(const std::string& lt_output) {
    std::vector<std::string> components;
    std::regex re("\\{[a-zA-Z]+\\s+([^}]+)\\}");
    std::smatch match;
    std::string::const_iterator searchStart(lt_output.cbegin());

    while (std::regex_search(searchStart, lt_output.cend(), match, re)) {
        if (match.size() > 1) {
            std::string objectName = match[1];
            components.push_back(objectName);
            std::cout << "Found object: " << objectName << std::endl;
        }
        searchStart = match.suffix().first;
    }
    return components;
}

bool ProcessGFiles::validateObject(const std::string& file_path, const std::string& object_name) {
    std::string mged_executable = MGED_EXECUTABLE_PATH;
    std::string command = mged_executable + " -c \"" + file_path + "\" l " + object_name;
    auto [output, error, return_code] = runCommand(command, 5);

    return return_code == 0;
}
