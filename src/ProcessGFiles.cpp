// ProcessGFiles.cpp

#include "ProcessGFiles.h"
#include "Model.h"
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

namespace fs = std::filesystem;

ProcessGFiles::ProcessGFiles(Model* model) : model(model) {
}

// worker function to process files from the queue
void ProcessGFiles::gFileWorker(std::queue<fs::path>& file_queue, const std::string& previews_folder) {
    while (true) {
        fs::path file_path;

        // Lock the queue and get the next file
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            if (file_queue.empty()) {
                return; // exit if the queue is empty
            }
            file_path = file_queue.front();
            file_queue.pop();
        }

        // Process the file
        try {
            processGFile(file_path, previews_folder);
            std::cout << "Processed .g file: " << file_path << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error processing file " << file_path << ": " << e.what() << std::endl;
        }
    }
}

// Function to execute the multi-threaded processing
void ProcessGFiles::executeMultiThreadedProcessing(const std::vector<std::string>& allGeometry, int num_workers) {
    // Fill the queue with `.g` files
    std::queue<fs::path> file_queue;
    for (const auto& file : allGeometry) {
        fs::path filePath(file);
        if (filePath.extension() == ".g") {
            file_queue.push(filePath);
        }
    }

    // Define the previews folder inside the hidden directory
    std::string previews_folder = model->getHiddenDirectoryPath() + "/previews";
    fs::create_directories(previews_folder);

    // Create worker threads
    std::vector<std::thread> threads;
    for (int i = 0; i < num_workers; ++i) {
        threads.emplace_back(&ProcessGFiles::gFileWorker, this, std::ref(file_queue), previews_folder);
    }

    // Wait for all threads to finish
    for (auto& thread : threads) {
        thread.join();
    }

    std::cout << "All files processed." << std::endl;
}

// Execute a system command and capture both stdout and stderr with timeout and exit code
std::tuple<std::string, std::string, int> ProcessGFiles::runCommand(const std::string& command, int timeout_seconds) {
    std::array<char, 256> buffer;
    std::string output;

    // Open a pipe to the command
    FILE* pipe = popen((command + " 2>&1").c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    auto start_time = std::chrono::steady_clock::now();

    // Read the output
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        output += buffer.data();

        // Check for timeout
        auto elapsed = std::chrono::steady_clock::now() - start_time;
        if (std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() > timeout_seconds) {
            pclose(pipe);
            throw std::runtime_error("Command timed out");
        }
    }

    // Get the exit code
    int return_code = pclose(pipe);

    return std::make_tuple(output, "", return_code);
}

// Parse the output of the 'tops' command to extract valid object names
std::vector<std::string> ProcessGFiles::parseTopsOutput(const std::string& tops_output) {
    std::vector<std::string> tops;
    std::istringstream iss(tops_output);
    for (std::string line; std::getline(iss, line);) {
        // Trim whitespace from the line
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.empty()) {
            continue;
        }

        // Remove any trailing slashes or colons
        if (!line.empty() && (line.back() == '/' || line.back() == ':')) {
            line.pop_back();
        }

        // Remove any non-alphanumeric characters from the object name
        line.erase(std::remove_if(line.begin(), line.end(), [](char c) {
                       return !std::isalnum(c) && c != '_' && c != '.';
                   }), line.end());

        if (!line.empty()) {
            tops.push_back(line);
        }
    }
    return tops;
}

// Validate if an object exists in the .g file
bool ProcessGFiles::validateObject(const std::string& file_path, const std::string& object_name) {
    std::string mged_executable = MGED_EXECUTABLE_PATH;
    std::string command = mged_executable + " -c \"" + file_path + "\" l " + object_name;
    auto [output, error, return_code] = runCommand(command, 5); // 5 seconds timeout

    // Log the output for debugging
    std::cout << "Validating object '" << object_name << "' in file '" << file_path << "'\n";
    std::cout << "Command: " << command << "\n";
    if (!output.empty()) {
        std::cout << "Output: " << output << "\n";
    }
    std::cout << "Return Code: " << return_code << "\n";

    // If exit code is 0, the object exists
    return return_code == 0;
}

// Process a single .g file and store metadata and thumbnails
void ProcessGFiles::processGFile(const fs::path& file_path, const std::string& previews_folder) {
    std::string mged_executable = MGED_EXECUTABLE_PATH;
    std::string rt_executable = RT_EXECUTABLE_PATH;

    std::string file_path_str = file_path.string();

    try {
        std::string model_short_name = file_path.stem().string();
        int modelId = model->hashModel(file_path.string());

        std::cout << "Processing file: " << file_path << "\n";

        // Ensure the previews folder exists
        fs::create_directories(previews_folder);

        // Retrieve or create ModelData
        ModelData modelData;
        if (model->modelExists(modelId)) {
            modelData = model->getModelById(modelId);
        } else {
            modelData.id = modelId;
            modelData.short_name = model_short_name;
            modelData.primary_file = file_path.filename().string();
            modelData.file_path = file_path.string();
            modelData.library_name = ""; // Set as needed
            model->insertModel(modelId, modelData);
        }

        // Extract title
        std::string title_command = mged_executable + " -c \"" + file_path_str + "\" title";
        auto [title_output, title_error, title_return_code] = runCommand(title_command);
        std::string title = !title_output.empty() ? title_output : title_error;
        title = title.empty() ? "Unknown" : title;
        title.erase(title.find_last_not_of(" \n\r\t") + 1);

        std::cout << "Title extracted: " << title << "\n";

        // update title in ModelData
        modelData.title = title;

        // Get top-level objects
        std::string tops_command = mged_executable + " -c \"" + file_path_str + "\" tops";
        auto [tops_output, tops_error, tops_return_code] = runCommand(tops_command);

        // Log the output of 'tops' command
        std::cout << "Tops command output:\n";
        if (!tops_output.empty()) {
            std::cout << tops_output << "\n";
        }

        std::string tops_result = !tops_output.empty() ? tops_output : tops_error;

        // Parse and validate tops elements
        std::vector<std::string> tops = parseTopsOutput(tops_result);

        // Determine the object to raytrace
        std::vector<std::string> objects_to_try = {"all", "all.g", model_short_name, model_short_name + ".g", model_short_name + ".c"};
        objects_to_try.insert(objects_to_try.end(), tops.begin(), tops.end());

        // Validate objects before attempting to raytrace
        std::vector<std::string> valid_objects;
        for (const auto& obj : objects_to_try) {
            if (validateObject(file_path.string(), obj)) {
                valid_objects.push_back(obj);
            } else {
                std::cerr << "Object does not exist in file: " << obj << std::endl;
            }
        }

        // If no valid objects found, log and return
        if (valid_objects.empty()) {
            std::cerr << "No valid objects to raytrace in file: " << file_path << "\n";
            // Update the model in the database
            model->updateModel(modelId, modelData);
            return;
        }

        // define the PNG file path in the previews folder
        std::string png_file = previews_folder + "/" + model_short_name + ".png";

        // ensure the directory for the png file exists
        fs::create_directories(fs::path(png_file).parent_path());

        std::string rt_command_template = rt_executable + " -s512 -o \"" + png_file + "\" \"" + file_path_str + "\" ";

        bool raytrace_successful = false;
        for (const auto& obj : valid_objects) {
            std::string rt_command = rt_command_template + obj;
            try {
                std::cout << "Attempting to raytrace object: " << obj << "\n";
                std::cout << "Command: " << rt_command << "\n";

                auto [rt_output, rt_error, rt_return_code] = runCommand(rt_command, 30); // 30 seconds timeout

                // log raytrace output
                if (!rt_output.empty()) {
                    std::cout << "Raytrace output:\n" << rt_output << "\n";
                }
                std::cout << "Raytrace Return Code: " << rt_return_code << "\n";

                std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Ensure file is written properly
                if (fs::exists(png_file) && fs::file_size(png_file) > 0) {
                    raytrace_successful = true;
                    std::cout << "Successfully created preview for: " << model_short_name << std::endl;

                    // Read the thumbnail data
                    std::ifstream thumbnail_file(png_file, std::ios::binary);
                    std::vector<char> thumbnail_data((std::istreambuf_iterator<char>(thumbnail_file)), std::istreambuf_iterator<char>());
                    thumbnail_file.close();

                    // Store the thumbnail in the ModelData
                    modelData.thumbnail = thumbnail_data;

                    // Optionally delete the PNG file from the previews folder
                    fs::remove(png_file);

                    break;
                } else {
                    std::cerr << "Raytrace did not produce a valid image for object: " << obj << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Raytrace failed for '" << obj << "', trying next object..." << std::endl;
                std::cerr << "Error: " << e.what() << std::endl;
            }
        }

        if (!raytrace_successful) {
            std::cerr << "Raytrace failed for all possible objects in " << model_short_name << "." << std::endl;
        }

        model->updateModel(modelId, modelData);

    } catch (const std::exception& e) {
        std::cerr << "Error processing file " << file_path << ": " << e.what() << std::endl;
    }
}
