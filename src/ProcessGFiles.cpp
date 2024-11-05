#include "ProcessGFiles.h"

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

bool ProcessGFiles::isModelProcessed(int modelId) {
  ModelData existingModel = model->getModelById(modelId);
  // Check if the model exists and has a thumbnail (or any other criteria)
  return existingModel.id == modelId && !existingModel.thumbnail.empty();
}

void ProcessGFiles::extractTitle(ModelData& modelData,
                                 const std::string& file_path) {
  std::string mged_executable = MGED_EXECUTABLE_PATH;
  std::string title_command =
      mged_executable + " -c \"" + file_path + "\" title";
  auto [title_output, title_error, title_return_code] =
      runCommand(title_command);
  std::string title = !title_output.empty() ? title_output : title_error;
  title = title.empty() ? "Unknown" : title;
  title.erase(title.find_last_not_of(" \n\r\t") + 1);

  debugPrint("Title extracted: " + title);

  modelData.title = title;
}

void ProcessGFiles::extractObjects(ModelData& modelData,
                                   const std::string& file_path) {
  std::string mged_executable = MGED_EXECUTABLE_PATH;

  // Set the maximum depth for traversal
  int max_depth = 3;

  // Use BFS to traverse the object hierarchy
  // Queue now includes the current depth
  std::queue<std::tuple<std::string, int, int>>
      object_queue;  // (object_name, parent_object_id, depth)
  std::set<std::string> processed_objects;

  // Get top-level objects
  std::string tops_command =
      mged_executable + " -c \"" + file_path + "\" tops -n";
  debugPrint("Executing tops command: " + tops_command);
  auto [tops_output, tops_error, tops_return_code] = runCommand(tops_command);
  std::string tops_result = !tops_output.empty() ? tops_output : tops_error;
  debugPrint("tops command output:\n" + tops_result);
  debugPrint("tops command return code: " + std::to_string(tops_return_code));

  // Parse the tops output
  std::vector<std::string> tops_elements = parseTopsOutput(tops_result);
  std::string tops_elements_str;
  for (const auto& elem : tops_elements) {
    tops_elements_str += elem + " ";
  }
  debugPrint("Parsed top-level elements: " + tops_elements_str);

  // Insert top-level objects
  for (const auto& top_element : tops_elements) {
    if (processed_objects.find(top_element) != processed_objects.end()) {
      debugPrint("Already processed top element: " + top_element);
      continue;
    }

    debugPrint("Inserting top-level object: " + top_element);

    ObjectData objectData;
    objectData.model_id = modelData.id;
    objectData.name = top_element;
    objectData.parent_object_id = -1;  // No parent for top-level objects
    objectData.is_selected = true;     // Default selection state

    int objectId = model->insertObject(objectData);
    if (objectId != -1) {
      debugPrint("Inserted object '" + top_element + "' with ID " +
                 std::to_string(objectId));
      int initial_depth = 1;  // Depth of top-level objects is 1
      object_queue.emplace(top_element, objectId, initial_depth);
      processed_objects.insert(top_element);
    } else {
      debugError("Failed to insert object: " + top_element);
    }
  }

  // Process the queue with depth limit
  while (!object_queue.empty()) {
    auto [current_object, parent_object_id, current_depth] =
        object_queue.front();
    object_queue.pop();

    debugPrint("Processing object: " + current_object +
               ", parent ID: " + std::to_string(parent_object_id) +
               ", depth: " + std::to_string(current_depth));

    // Check if we've reached the maximum depth
    if (current_depth >= max_depth) {
      debugPrint("Reached maximum depth for object: " + current_object);
      continue;
    }

    // Get child objects
    std::string lt_command =
        mged_executable + " -c \"" + file_path + "\" lt " + current_object;
    debugPrint("Executing lt command: " + lt_command);
    auto [lt_output, lt_error, lt_return_code] = runCommand(lt_command);
    std::string lt_result = !lt_output.empty() ? lt_output : lt_error;
    debugPrint("lt command output:\n" + lt_result);
    debugPrint("lt command return code: " + std::to_string(lt_return_code));

    // Parse the lt output
    std::vector<std::string> child_objects = parseLtOutput(lt_result);
    std::string child_objects_str;
    for (const auto& child : child_objects) {
      child_objects_str += child + " ";
    }
    debugPrint("Parsed child objects of " + current_object + ": " +
               child_objects_str);

    for (const auto& child_object : child_objects) {
      if (processed_objects.find(child_object) != processed_objects.end()) {
        debugPrint("Already processed child object: " + child_object);
        continue;
      }

      debugPrint("Inserting child object: " + child_object +
                 ", parent ID: " + std::to_string(parent_object_id));

      ObjectData objectData;
      objectData.model_id = modelData.id;
      objectData.name = child_object;
      objectData.parent_object_id = parent_object_id;
      objectData.is_selected = true;  // Default selection state

      int objectId = model->insertObject(objectData);

      if (objectId != -1) {
        debugPrint("Inserted object '" + child_object + "' with ID " +
                   std::to_string(objectId));
        int child_depth = current_depth + 1;
        object_queue.emplace(child_object, objectId, child_depth);
        processed_objects.insert(child_object);
      } else {
        debugError("Failed to insert object: " + child_object);
      }
    }
  }
}

void ProcessGFiles::generateThumbnail(ModelData& modelData,
                                      const std::string& file_path,
                                      const std::string& previews_folder) {
  std::string rt_executable = RT_EXECUTABLE_PATH;
  std::string model_short_name = fs::path(file_path).stem().string();

  // Determine objects to try for raytracing
  std::vector<std::string> objects_to_try = {"all", "all.g", model_short_name,
                                             model_short_name + ".g",
                                             model_short_name + ".c"};

  // Validate objects
  std::vector<std::string> valid_objects;
  for (const auto& obj : objects_to_try) {
    if (validateObject(file_path, obj)) {
      valid_objects.push_back(obj);
    }
  }

  if (valid_objects.empty()) {
    debugError("No valid objects to raytrace in file: " + file_path + "\n");
    return;
  }

  // Generate thumbnail
  std::string png_file = previews_folder + "/" + model_short_name + ".png";
  fs::create_directories(fs::path(png_file).parent_path());
  std::string rt_command_template =
      rt_executable + " -s512 -o \"" + png_file + "\" \"" + file_path + "\" ";

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
        std::vector<char> thumbnail_data(
            (std::istreambuf_iterator<char>(thumbnail_file)),
            std::istreambuf_iterator<char>());
        thumbnail_file.close();

        modelData.thumbnail = thumbnail_data;

        // Optionally delete the PNG file
        fs::remove(png_file);
        break;
      }
    } catch (const std::exception& e) {
      debugError("Raytrace failed for '" + obj + "': " + e.what());
    }
  }

  if (!raytrace_successful) {
    debugError("Raytrace failed for all possible objects in " +
               model_short_name + ".");
  }
}

void ProcessGFiles::processGFile(const fs::path& file_path,
                                 const std::string& previews_folder,
                                 const std::string& library_name) {
  try {
    std::string model_short_name = file_path.stem().string();
    int modelId = model->hashModel(file_path.string());

    if (isModelProcessed(modelId)) {
      debugPrint("Model already processed: " + model_short_name);
      return;
    }

    // Prepare ModelData
    ModelData modelData;
    modelData.id = modelId;
    modelData.short_name = model_short_name;
    modelData.primary_file = file_path.filename().string();
    modelData.file_path = file_path.string();
    modelData.library_name = library_name;
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
    debugError("Error processing file " + file_path.string() + ": " + e.what());
  }
}

std::tuple<std::string, std::string, int> ProcessGFiles::runCommand(
    const std::string& command, int timeout_seconds) {
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
    if (std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() >
        timeout_seconds) {
      pclose(pipe);
      throw std::runtime_error("Command timed out");
    }
  }

  int return_code = pclose(pipe);

  return {output, "", return_code};
}

std::vector<std::string> ProcessGFiles::parseTopsOutput(
    const std::string& tops_output) {
  std::vector<std::string> tops_elements;
  std::istringstream iss(tops_output);
  for (std::string line; std::getline(iss, line);) {
    // Trim whitespace
    line.erase(0, line.find_first_not_of(" \t\r\n"));
    line.erase(line.find_last_not_of(" \t\r\n") + 1);

    if (!line.empty()) {
      tops_elements.push_back(line);
      debugPrint("Found top-level object: " + line);
    }
  }
  return tops_elements;
}

std::vector<std::string> ProcessGFiles::parseLtOutput(
    const std::string& lt_output) {
  std::vector<std::string> components;
  std::regex re("\\{[a-zA-Z]+\\s+([^}]+)\\}");
  std::smatch match;
  std::string::const_iterator searchStart(lt_output.cbegin());

  while (std::regex_search(searchStart, lt_output.cend(), match, re)) {
    if (match.size() > 1) {
      std::string objectName = match[1];
      components.push_back(objectName);
      debugPrint("Found object: " + objectName);
    }
    searchStart = match.suffix().first;
  }
  return components;
}

bool ProcessGFiles::validateObject(const std::string& file_path,
                                   const std::string& object_name) {
  std::string mged_executable = MGED_EXECUTABLE_PATH;
  std::string command =
      mged_executable + " -c \"" + file_path + "\" l " + object_name;
  auto [output, error, return_code] = runCommand(command, 5);

  return return_code == 0;
}
