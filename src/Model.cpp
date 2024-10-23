// Model.cpp

#include "Model.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

Model::Model(const std::string& libraryPath) : db(nullptr) {
    // create a hidden directory inside the library path
    fs::path hiddenDir = fs::path(libraryPath) / ".cadventory";
    hiddenDirPath = hiddenDir.string();
    if (!fs::exists(hiddenDir)) {
        fs::create_directory(hiddenDir);
    }

    // set the database path inside the hidden directory
    dbPath = (hiddenDir / "metadata.db").string();

    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Can't open database at " << dbPath << ": " << sqlite3_errmsg(db) << std::endl;
    } else {
        std::cout << "Opened database at " << dbPath << " successfully" << std::endl;
        createTables();
    }
}

Model::~Model() {
    if (db) {
        sqlite3_close(db);
    }
}

bool Model::createTables() {
    std::string sqlModels = R"(
        CREATE TABLE IF NOT EXISTS models (
            id INTEGER PRIMARY KEY,
            short_name TEXT NOT NULL,
            primary_file TEXT,
            override_info TEXT,
            title TEXT,
            thumbnail BLOB,
            author TEXT,
            file_path TEXT,
            library_name TEXT
        );
    )";

    return executeSQL(sqlModels);
}

bool Model::insertModel(int id, const ModelData& modelData) {
    std::string sql = R"(
        INSERT OR IGNORE INTO models
        (id, short_name, primary_file, override_info, title, thumbnail, author, file_path, library_name)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt;
    std::lock_guard<std::mutex> lock(db_mutex);

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);
        sqlite3_bind_text(stmt, 2, modelData.short_name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, modelData.primary_file.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, modelData.override_info.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, modelData.title.c_str(), -1, SQLITE_STATIC);

        if (!modelData.thumbnail.empty()) {
            sqlite3_bind_blob(stmt, 6, modelData.thumbnail.data(), static_cast<int>(modelData.thumbnail.size()), SQLITE_STATIC);
        } else {
            sqlite3_bind_null(stmt, 6);
        }

        sqlite3_bind_text(stmt, 7, modelData.author.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 8, modelData.file_path.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 9, modelData.library_name.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Insert model failed: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }
        sqlite3_finalize(stmt);
        return true;
    } else {
        std::cerr << "SQL error in insertModel: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
}

bool Model::updateModel(int id, const ModelData& modelData) {
    std::string sql = R"(
        UPDATE models SET
            short_name = ?,
            primary_file = ?,
            override_info = ?,
            title = ?,
            thumbnail = ?,
            author = ?,
            file_path = ?,
            library_name = ?
        WHERE id = ?;
    )";

    sqlite3_stmt* stmt;
    std::lock_guard<std::mutex> lock(db_mutex);

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, modelData.short_name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, modelData.primary_file.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, modelData.override_info.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, modelData.title.c_str(), -1, SQLITE_STATIC);

        if (!modelData.thumbnail.empty()) {
            sqlite3_bind_blob(stmt, 5, modelData.thumbnail.data(), static_cast<int>(modelData.thumbnail.size()), SQLITE_STATIC);
        } else {
            sqlite3_bind_null(stmt, 5);
        }

        sqlite3_bind_text(stmt, 6, modelData.author.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 7, modelData.file_path.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 8, modelData.library_name.c_str(), -1, SQLITE_STATIC);

        sqlite3_bind_int(stmt, 9, id);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Update model failed: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }
        sqlite3_finalize(stmt);
        return true;
    } else {
        std::cerr << "SQL error in updateModel: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
}

bool Model::deleteModel(int id) {
    std::string sql = "DELETE FROM models WHERE id = ?;";
    sqlite3_stmt* stmt;
    std::lock_guard<std::mutex> lock(db_mutex);

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Delete model failed: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }
        sqlite3_finalize(stmt);
        return true;
    } else {
        std::cerr << "SQL error in deleteModel: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
}

bool Model::modelExists(int id) {
    std::string sql = "SELECT COUNT(*) FROM models WHERE id = ?;";
    sqlite3_stmt* stmt;
    int count = 0;
    std::lock_guard<std::mutex> lock(db_mutex);

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    } else {
        std::cerr << "SQL error in modelExists: " << sqlite3_errmsg(db) << std::endl;
    }

    return count > 0;
}

std::vector<ModelData> Model::getModels() {
    std::vector<ModelData> models;
    std::string sql = R"(
        SELECT id, short_name, primary_file, override_info, title, thumbnail, author, file_path, library_name
        FROM models;
    )";
    sqlite3_stmt* stmt;
    std::lock_guard<std::mutex> lock(db_mutex);

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            ModelData model;
            model.id = sqlite3_column_int(stmt, 0);
            model.short_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            model.primary_file = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            model.override_info = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            model.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

            const void* blob = sqlite3_column_blob(stmt, 5);
            int blob_size = sqlite3_column_bytes(stmt, 5);
            if (blob && blob_size > 0) {
                model.thumbnail.assign(static_cast<const char*>(blob), static_cast<const char*>(blob) + blob_size);
            }

            model.author = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            model.file_path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            model.library_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));

            models.push_back(model);
        }
        sqlite3_finalize(stmt);
    } else {
        std::cerr << "Failed to select models: " << sqlite3_errmsg(db) << std::endl;
    }
    return models;
}

ModelData Model::getModelById(int id) {
    std::string sql = R"(
        SELECT id, short_name, primary_file, override_info, title, thumbnail, author, file_path, library_name
        FROM models WHERE id = ?;
    )";
    sqlite3_stmt* stmt;
    ModelData model;
    std::lock_guard<std::mutex> lock(db_mutex);

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            model.id = sqlite3_column_int(stmt, 0);
            model.short_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            model.primary_file = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            model.override_info = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            model.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

            const void* blob = sqlite3_column_blob(stmt, 5);
            int blob_size = sqlite3_column_bytes(stmt, 5);
            if (blob && blob_size > 0) {
                model.thumbnail.assign(static_cast<const char*>(blob), static_cast<const char*>(blob) + blob_size);
            }

            model.author = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            model.file_path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            model.library_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        }
        sqlite3_finalize(stmt);
    } else {
        std::cerr << "Failed to select model: " << sqlite3_errmsg(db) << std::endl;
    }

    return model;
}

int Model::hashModel(const std::string& modelDir) {
    std::ifstream file(modelDir, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Could not open file for hashing: " << modelDir << std::endl;
        return 0;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string fileContents = buffer.str();

    std::hash<std::string> hasher;
    return static_cast<int>(hasher(fileContents));
}

void Model::printModel(int modelId) {
    ModelData model = getModelById(modelId);
    std::cout << "Model ID: " << model.id << std::endl;
    std::cout << "Short Name: " << model.short_name << std::endl;
    std::cout << "Primary File: " << model.primary_file << std::endl;
    std::cout << "Override Info: " << model.override_info << std::endl;
    std::cout << "Title: " << model.title << std::endl;
    std::cout << "Author: " << model.author << std::endl;
    std::cout << "File Path: " << model.file_path << std::endl;
    std::cout << "Library Name: " << model.library_name << std::endl;

    if (!model.thumbnail.empty()) {
        std::cout << "Thumbnail: [binary data]" << std::endl;
    } else {
        std::cout << "Thumbnail: [none]" << std::endl;
    }
}

std::string Model::getHiddenDirectoryPath() const {
    return hiddenDirPath;
}

bool Model::executeSQL(const std::string& sql) {
    char* errMsg = nullptr;
    std::lock_guard<std::mutex> lock(db_mutex);
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error in executeSQL: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}
