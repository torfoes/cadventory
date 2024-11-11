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
std::vector<ObjectData> ProcessGFiles::extractObjects(
    const ModelData& modelData, const std::string& file_path,
    std::map<std::string, std::string>& parentRelations) {
  std::vector<ObjectData> objects;
  std::string mged_executable = MGED_EXECUTABLE_PATH;
  int max_depth = 3;

  std::queue<std::tuple<std::string, std::string, int>> object_queue;
  std::set<std::string> processed_objects;

  std::string tops_command =
      mged_executable + " -c \"" + file_path + "\" tops -n";
  auto [tops_output, tops_error, tops_return_code] = runCommand(tops_command);
  std::string tops_result = !tops_output.empty() ? tops_output : tops_error;
  std::vector<std::string> tops_elements = splitStringByWhitespace(tops_result);

  // Determine the selected object
  std::string model_short_name = modelData.short_name;
  std::vector<std::string> objects_to_try = {"all", "all.g", model_short_name,
                                             model_short_name + ".g",
                                             model_short_name + ".c"};
  std::string selected_object_name;

  // check if any objects_to_try are in tops_elements
  for (const auto& obj_name : objects_to_try) {
    if (std::find(tops_elements.begin(), tops_elements.end(), obj_name) !=
        tops_elements.end()) {
      selected_object_name = obj_name;
      break;
    }
  }

  // if no match, select the first top-level object
  if (selected_object_name.empty() && !tops_elements.empty()) {
    selected_object_name = tops_elements.front();
  }

  for (const auto& top_element : tops_elements) {
    if (processed_objects.count(top_element)) continue;

    ObjectData objectData;
    objectData.model_id = modelData.id;
    objectData.name = top_element;
    objectData.parent_object_id = -1;
    objectData.is_selected = (top_element == selected_object_name);

    objects.push_back(objectData);
    processed_objects.insert(top_element);
    object_queue.emplace(top_element, "", 1);

    parentRelations[top_element] = "";
  }

  // BFS traversal
  while (!object_queue.empty()) {
    auto [current_object, parent_name, current_depth] = object_queue.front();
    object_queue.pop();

    if (current_depth >= max_depth) continue;

    // Get child objects
    std::string lt_command =
        mged_executable + " -c \"" + file_path + "\" lt " + current_object;
    auto [lt_output, lt_error, lt_return_code] = runCommand(lt_command);
    std::string lt_result = !lt_output.empty() ? lt_output : lt_error;

    // Parse lt output
    std::vector<std::string> child_objects = parseLtOutput(lt_result);

    for (const auto& child_object : child_objects) {
      if (processed_objects.count(child_object)) continue;

      ObjectData objectData;
      objectData.model_id = modelData.id;
      objectData.name = child_object;
      objectData.parent_object_id = -1;
      objectData.is_selected = (child_object == selected_object_name);

      objects.push_back(objectData);
      processed_objects.insert(child_object);
      object_queue.emplace(child_object, current_object, current_depth + 1);

      parentRelations[child_object] = current_object;
    }
  }

  return objects;
}

void ProcessGFiles::generateThumbnail(ModelData& modelData,
                                      const std::string& file_path,
                                      const std::string& previews_folder,
                                      const std::string& selected_object_name) {
  std::string rt_executable = RT_EXECUTABLE_PATH;
  std::string model_short_name = fs::path(file_path).stem().string();

  QSettings settings;
  int timeLimit = settings.value("previewTimer", 30).toInt();

  if (selected_object_name.empty()) {
    std::cerr << "No valid object selected for raytrace in file: " << file_path
              << "\n";
    return;
  }

  // Generate thumbnail
  std::string png_file = previews_folder + "/" + model_short_name + ".png";
  fs::create_directories(fs::path(png_file).parent_path());
  std::string rt_command = rt_executable + " -s512 -o \"" + png_file + "\" \"" +
                           file_path + "\" " + selected_object_name;

  try {
    auto [rt_output, rt_error, rt_return_code] =
        runCommand(rt_command, timeLimit);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (fs::exists(png_file) && fs::file_size(png_file) > 0) {
      // Read thumbnail data
      std::ifstream thumbnail_file(png_file, std::ios::binary);
      std::vector<char> thumbnail_data(
          (std::istreambuf_iterator<char>(thumbnail_file)),
          std::istreambuf_iterator<char>());
      thumbnail_file.close();

      modelData.thumbnail = thumbnail_data;

      // Optionally delete the PNG file
      fs::remove(png_file);
    } else {
      std::cerr << "Raytrace failed for object: " << selected_object_name
                << "\n";
    }
  } catch (const std::exception& e) {
    std::cerr << "Raytrace failed for '" << selected_object_name
              << "': " << e.what() << "\n";
  }
}

void ProcessGFiles::processGFile(const fs::path& file_path,
                                 const std::string& previews_folder,
                                 const std::string& library_name
                                 ) {
  try {
    QSettings settings;
    bool previewFlag = settings.value("previewFlag", true).toBool();

    std::string model_short_name = file_path.stem().string();
    int modelId = model->hashModel(file_path.string());

        // Use getModelByFilePath to check if the model already exists
        ModelData existingModel = model->getModelByFilePath(file_path.string());

        if (existingModel.id != 0 && existingModel.is_processed) {
            std::cout << "Model already processed: " << model_short_name << "\n";
            return;
        }

        // Prepare ModelData (do not set id)
        ModelData modelData;
        modelData.short_name = model_short_name;
        modelData.primary_file = file_path.filename().string();
        modelData.file_path = file_path.string();
        modelData.library_name = "";
        modelData.is_selected = false;
        modelData.is_included = true; // Since we're processing included models

    // Extract title
    extractTitle(modelData, file_path.string());

    // Extract objects
    std::map<std::string, std::string>
        parentRelations;  // objectName -> parentName
    std::vector<ObjectData> objects =
        extractObjects(modelData, file_path.string(), parentRelations);

    // Determine selected object name
    std::string selected_object_name;
    for (const auto& obj : objects) {
      if (obj.is_selected) {
        selected_object_name = obj.name;
        break;
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
                return;
            }
            // After insertion, retrieve the model to get the assigned id
            ModelData insertedModel = model->getModelByFilePath(file_path.string());
            if (insertedModel.id == 0) {
                std::cerr << "Failed to retrieve inserted model: " << model_short_name << "\n";
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
            objData.model_id = modelData.id; // Ensure model_id is set correctly
            int objectId = model->insertObject(objData);
            objectNameToId[objData.name] = objectId;
        }

        // Update parent_object_id for each object
        for (const auto& objData : objects) {
            std::string parentName = parentRelations[objData.name];
            if (!parentName.empty()) {
                int objectId = objectNameToId[objData.name];
                int parentObjectId = objectNameToId[parentName];
                model->updateObjectParentId(objectId, parentObjectId);
            }
        }

    // Commit transaction
    model->commitTransaction();

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

// Function to split a string by whitespace
std::vector<std::string> ProcessGFiles::splitStringByWhitespace(
    const std::string& input) {
  std::istringstream stream(input);
  std::vector<std::string> tokens;
  std::string token;

  while (stream >> token) {
    tokens.push_back(token);
  }

  return tokens;
}

bool ProcessGFiles::validateObject(const std::string& file_path,
                                   const std::string& object_name) {
  std::string mged_executable = MGED_EXECUTABLE_PATH;
  std::string command =
      mged_executable + " -c \"" + file_path + "\" l " + object_name;
  auto [output, error, return_code] = runCommand(command, 5);

  return return_code == 0;
}

#include <iostream>  // Ensure this is included for logging

std::tuple<bool, std::string> ProcessGFiles::generateGistReport(const std::string& inputFilePath, const std::string& outputFilePath, const std::string& primary_obj, const std::string& label) {
    // log the function entry and input parameters
    std::cout << "generateGistReport called with:" << std::endl;
    std::cout << "  inputFilePath: " << inputFilePath << std::endl;
    std::cout << "  outputFilePath: " << outputFilePath << std::endl;
    std::cout << "  primary obj: " << primary_obj << std::endl;
    std::cout << "  label: " << label << std::endl;
    // check if input file exists
    QFileInfo inputFile(QString::fromStdString(inputFilePath));
    if (!inputFile.exists()) {
        std::cerr << "Input file does not exist: " << inputFilePath << std::endl;
        return {false, "Input file does not exist: " + inputFilePath};
    } else {
        std::cout << "Confirmed input file exists." << std::endl;
    }

  // construct the gist command
  std::string gistCommand = std::string(GIST_EXECUTABLE_PATH) + " \"" +
                            inputFilePath + "\" -o \"" + outputFilePath + "\"";

    if(!primary_obj.empty()){
        gistCommand += " -t \"" + primary_obj + "\"";
    }
    if(!label.empty()){
        gistCommand += " -c \"" + label + "\"";
    }
    std::cout << "Constructed gistCommand: " << gistCommand << std::endl;

    // execute the command
    auto [stdoutStr, stderrStr, returnCode] = runCommand(gistCommand, 999999);

  // log command execution results
  std::cout << "Command execution completed with returnCode: " << returnCode
            << std::endl;
  std::cout << "Standard Output:" << std::endl << stdoutStr << std::endl;
  std::cout << "Standard Error:" << std::endl << stderrStr << std::endl;

  // check for command execution errors
  if (returnCode != 0 || !stdoutStr.empty() || !stderrStr.empty()) {
    std::cerr << "Gist command failed with code " << returnCode << std::endl;
    std::string errorMsg = stderrStr.empty() ? stdoutStr : stderrStr;
    return {false, "Gist command failed with code " +
                       std::to_string(returnCode) + ": " + errorMsg};
  } else {
    std::cout << "Gist command executed successfully." << std::endl;
  }

  // check if the output file was generated
  QFileInfo outputFile(QString::fromStdString(outputFilePath));
  if (!outputFile.exists()) {
    std::cerr << "Output file not generated: " << outputFilePath << std::endl;
    return {false, stdoutStr};
  } else {
    std::cout << "Confirmed output file was generated successfully."
              << std::endl;
  }

  std::cout << "Gist report generation completed successfully for file: "
            << inputFilePath << std::endl;
  return {true, ""};
}
