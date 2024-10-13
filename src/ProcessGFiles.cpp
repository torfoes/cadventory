#include "ProcessGFiles.h"
#include "Model.h"
#include "FilesystemIndexer.h"
#include <iostream>
#include <fstream>
#include <queue>
#include <thread>
#include <vector>
#include <mutex>
#include <cstdlib>
#include <cstdio>
#include <array>
#include <sstream>
#include <chrono>
#include <filesystem>
#include <QThread>

namespace fs = std::filesystem;

using namespace std;

mutex db_mutex; // To protect database operations
mutex queue_mutex; // To protect the file queue
vector<pair<string, string>> failed_files;


// ProcessGFiles constructor
ProcessGFiles::ProcessGFiles() {
    
}

// Worker function to process files from the queue
void ProcessGFiles::gFileWorker(std::queue<fs::path>& file_queue) {
    while (true) {
        fs::path file_path;

        // Lock the queue and get the next file
        {
            lock_guard<mutex> lock(queue_mutex);
            if (file_queue.empty()) {
                return; // Exit if the queue is empty
            }
            file_path = file_queue.front();
            file_queue.pop();
        }

        // Process the file
        try {
            processGFile(file_path);
            cout << "Processed .g file: " << file_path << endl;
        } catch (const exception& e) {
            cerr << "Error processing file " << file_path << ": " << e.what() << endl;
        }
    }
}

// Function to execute the multi-threaded processing
void ProcessGFiles::executeMultiThreadedProcessing(const std::vector<std::string>& allGeometry, int num_workers) {
    // Fill the queue with `.g` files
    std::queue<fs::path> file_queue;
    for (const auto& file : allGeometry) {
        std::filesystem::path filePath(file);
        if (filePath.extension() == ".g") {
            file_queue.push(filePath);
        }
    }

    // Create worker threads
    vector<thread> threads;
    for (int i = 0; i < num_workers; ++i) {
        threads.emplace_back(&ProcessGFiles::gFileWorker, this, std::ref(file_queue));
    }

    // Wait for all threads to finish
    for (auto& thread : threads) {
        thread.join();
    }

    cout << "All files processed." << endl;
}


// Execute a system command and capture both stdout and stderr
std::pair<string, string> ProcessGFiles::runCommand(const std::string& command) {
    array<char, 128> buffer;
    string stdout_result;
    string stderr_result;

    // Capture both stdout and stderr using redirection
    FILE* pipe = popen((command + " 2>&1").c_str(), "r");
    if (!pipe) {
        throw runtime_error("popen() failed!");
    }

    // Read command output
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        stdout_result += buffer.data();
    }

    // Get the return code of the command
    int return_code = pclose(pipe);
    if (return_code != 0) {
        // If the return code is non-zero, treat the output as an error (stderr)
        stderr_result = stdout_result;
        stdout_result.clear();
    }

    return {stdout_result, stderr_result};
}

// Extract metadata and generate previews
void ProcessGFiles::processGFile(const fs::path& file_path) {
    try {
        string model_short_name = file_path.stem().string();

        // Extract title using updated runCommand
        string title_command = "/mnt/c/Users/Agoni/OneDrive/Desktop/AdamsCode/brlcad/build/bin/mged -c " + file_path.string() + " title";
        auto title_result = runCommand(title_command);
        string title = !title_result.first.empty() ? title_result.first : title_result.second;
        title = title.empty() ? "Unknown" : title;

        // Get tops level objects
        string tops_command = "/mnt/c/Users/Agoni/OneDrive/Desktop/AdamsCode/brlcad/build/bin/mged -c " + file_path.string() + " tops";
        auto tops_result = runCommand(tops_command);
        string tops_output = !tops_result.first.empty() ? tops_result.first : tops_result.second;

        // Process tops elements
        vector<string> tops;
        istringstream iss(tops_output);
        for (string line; getline(iss, line);) {
            if (!line.empty()) {
                tops.push_back(line);
            }
        }

        // Determine the object to raytrace
        vector<string> objects_to_try = {"all", "all.g", model_short_name, model_short_name + ".g", model_short_name + ".c"};
        objects_to_try.insert(objects_to_try.end(), tops.begin(), tops.end());

        string png_file =  model_short_name + ".png";
        string rt_command_template = "/mnt/c/Users/Agoni/OneDrive/Desktop/AdamsCode/brlcad/build/bin/rt -s2048 -o " + png_file + " " + file_path.string() + " ";

        bool raytrace_successful = false;
        for (const auto& obj : objects_to_try) {
            string rt_command = rt_command_template + obj;
            try {
                auto [rt_output, rt_error] = runCommand(rt_command);
                this_thread::sleep_for(chrono::milliseconds(100));  // Ensure file is written properly
                if (fs::exists(png_file) && fs::file_size(png_file) > 0) {
                    raytrace_successful = true;
                    cout << "Successfully created preview for: " << model_short_name << endl;
                    break;
                }
            } catch (const exception& e) {
                cerr << "Raytrace failed for '" << obj << "', trying next object..." << endl;
            }
        }

        if (!raytrace_successful) {
            cerr << "Raytrace failed for all possible objects in " << model_short_name << "." << endl;
            failed_files.push_back({title, file_path.string()});
        }

    } catch (const exception& e) {
        cerr << "Error processing file " << file_path << ": " << e.what() << endl;
    }
}
