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

mutex db_mutex;
vector<pair<string, string>> failed_files;

// ProcessGFiles constructor
ProcessGFiles::ProcessGFiles(const std::string& dbPath) : dbPath(dbPath) {
    // Initialize the database if needed
    createDatabase();
}

// Connect to SQLite database or create it
void ProcessGFiles::createDatabase() {
    sqlite3* db;
    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc) {
        cerr << "Cannot open database: " << sqlite3_errmsg(db) << endl;
        return;
    }

    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS model_metadata (
            model_short_name TEXT,
            model_long_name TEXT,
            model_aliases TEXT,
            model_type TEXT,
            organizational_owner TEXT,
            model_source TEXT,
            filesystem_location TEXT,
            model_creation_date TEXT,
            last_modification_date TEXT,
            modeler TEXT,
            file_owner TEXT,
            model_suitability TEXT,
            classification TEXT,
            conversions TEXT,
            exists_photography TEXT,
            exists_drawings TEXT,
            exists_measurements TEXT,
            unaccounted_files TEXT,
            accounted_files TEXT,
            model_image TEXT
        );
    )";

    char* errMsg = nullptr;
    rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        cerr << "SQL error: " << errMsg << endl;
        sqlite3_free(errMsg);
    }

    sqlite3_close(db);
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

        // Extract title
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

        string png_file = "../previews/" + model_short_name + ".png";
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

        // Insert metadata into the database
        lock_guard<mutex> lock(db_mutex);
        sqlite3* db;
        sqlite3_open(dbPath.c_str(), &db);
        string sql = "INSERT INTO model_metadata (model_short_name, model_long_name, filesystem_location, model_creation_date, last_modification_date, file_owner, model_image) VALUES (?, ?, ?, ?, ?, ?, ?);";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, model_short_name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, title.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, file_path.string().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, "", -1, SQLITE_STATIC);  // Placeholder for creation date
        sqlite3_bind_text(stmt, 5, "", -1, SQLITE_STATIC);  // Placeholder for modification date
        sqlite3_bind_text(stmt, 6, "", -1, SQLITE_STATIC);  // Placeholder for file owner
        sqlite3_bind_text(stmt, 7, raytrace_successful ? png_file.c_str() : nullptr, -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        sqlite3_close(db);

    } catch (const exception& e) {
        cerr << "Error processing file " << file_path << ": " << e.what() << endl;
    }
}
