
#include "Model.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>


Model::Model(const std::string& path) : db(nullptr), dbPath(path) {
  if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
    std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
  } else {
    // init on instantiation
    std::cout << "Opened database successfully" << std::endl;
    createTable();
  }
}

Model::~Model() {
  if (db) {
    sqlite3_close(db);
  }
}

bool Model::createTable() {
    std::string sqlModels = R"(
        CREATE TABLE IF NOT EXISTS models (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            short_name TEXT NOT NULL,
            primary_file TEXT,
            override_info TEXT
        );
    )";
    std::string sqlTags = R"(
        CREATE TABLE IF NOT EXISTS tags (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE
        );
    )";
    std::string sqlModelTags = R"(
        CREATE TABLE IF NOT EXISTS model_tags (
            model_id INTEGER NOT NULL,
            tag_id INTEGER NOT NULL,
            PRIMARY KEY (model_id, tag_id),
            FOREIGN KEY (model_id) REFERENCES models(id) ON DELETE CASCADE,
            FOREIGN KEY (tag_id) REFERENCES tags(id) ON DELETE CASCADE
        );
    )";

    return executeSQL(sqlModels) && executeSQL(sqlTags) && executeSQL(sqlModelTags);
}

bool Model::insertModel(const std::string& filePath, const std::string& shortName, const std::string& primaryFile, const std::string& overrides) {
  int id = hashModel(filePath);
  return insertModel(id, shortName, primaryFile, overrides);
}

bool Model::insertModel(int id, const std::string& shortName, const std::string& primaryFile, const std::string& overrides) {
  std::string sql = "INSERT INTO models (id, short_name, primary_file, override_info) VALUES (?, ?, ?, ?);";
  // std::cout << "Inserting model: " << id << " " << shortName << " " << primaryFile << " " << overrides;


  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_int(stmt, 1, id);
    sqlite3_bind_text(stmt, 2, shortName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, primaryFile.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, overrides.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
      // std::cerr << " Insert model failed: " << sqlite3_errmsg(db) << std::endl;
      sqlite3_finalize(stmt);
      return false;
    }
    sqlite3_finalize(stmt);
    // std::cout << " Model inserted" << std::endl;
    return true;
  } else {
    std::cout << " SQL error: " << sqlite3_errmsg(db) << std::endl;
  }

  return false;
}

std::vector<ModelData> Model::getModels() {
  std::vector<ModelData> models;
  std::string sql = "SELECT id, short_name, primary_file, override_info FROM models;";
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      models.push_back({
          sqlite3_column_int(stmt, 0),
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)),
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3))
        });
    }
    sqlite3_finalize(stmt);
  } else {
    std::cerr << "Failed to select models: " << sqlite3_errmsg(db) << std::endl;
  }
  return models;
}

ModelData Model::getModelById(int id) {
  std::string sql = "SELECT id, short_name, primary_file, override_info FROM models WHERE id = ?;";
  sqlite3_stmt* stmt;
  ModelData model = {0, "", "", ""};

  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_int(stmt, 1, id);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      model = {
        sqlite3_column_int(stmt, 0),
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)),
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3))
      };
    }
    sqlite3_finalize(stmt);
  } else {
    std::cerr << "Failed to select model: " << sqlite3_errmsg(db) << std::endl;
  }

  return model;
}

bool Model::updateModel(int id, const std::string& shortName, const std::string& primaryFile, const std::string& overrides) {
  std::string sql = "UPDATE models SET short_name = ?, primary_file = ?, override_info = ? WHERE id = ?;";
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, shortName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, primaryFile.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, overrides.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
      std::cerr << "Update model failed: " << sqlite3_errmsg(db) << std::endl;
      sqlite3_finalize(stmt);
      return false;
    }
    sqlite3_finalize(stmt);
    return true;
  } else {
    std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    return false;
  }
}

bool Model::deleteModel(int id) {
  std::string sql = "DELETE FROM models WHERE id = ?;";
  sqlite3_stmt* stmt;
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
    std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    return false;
  }
}

bool Model::addTagToModel(int modelId, const std::string& tagName) {
    // Insert the tag if it doesn't already exist
    std::string sqlTagInsert = "INSERT OR IGNORE INTO tags (name) VALUES (?);";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sqlTagInsert.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, tagName.c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    } else {
        std::cerr << "Failed to insert tag: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    // Link the tag to the model
    std::string sqlTagId = "SELECT id FROM tags WHERE name = ?;";
    if (sqlite3_prepare_v2(db, sqlTagId.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, tagName.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int tagId = sqlite3_column_int(stmt, 0);
            sqlite3_finalize(stmt);
            
            std::string sqlLink = "INSERT INTO model_tags (model_id, tag_id) VALUES (?, ?);";
            if (sqlite3_prepare_v2(db, sqlLink.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_int(stmt, 1, modelId);
                sqlite3_bind_int(stmt, 2, tagId);
                sqlite3_step(stmt);
                sqlite3_finalize(stmt);
                return true;
            }
        }
        sqlite3_finalize(stmt);
    }
    
    std::cerr << "Failed to add tag to model: " << sqlite3_errmsg(db) << std::endl;
    return false;
}

std::vector<std::string> Model::getTagsForModel(int modelId) {
    std::vector<std::string> tags;
    std::string sql = "SELECT name FROM tags t JOIN model_tags mt ON t.id = mt.tag_id WHERE mt.model_id = ?;";
    sqlite3_stmt* stmt = nullptr;  // Ensure stmt is initialized to nullptr
    int rc;

    // std::cout << "Getting tags for model: " << modelId << std::endl;

    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        // Bind the modelId to the first parameter (?)
        sqlite3_bind_int(stmt, 1, modelId);

        // Fetch rows from the query result
        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            const char* tagText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));

            // Ensure tagText is not NULL
            if (tagText) {
                tags.push_back(tagText);
            } else {
                std::cerr << "Null value encountered for tag name." << std::endl;
            }
        }

        if (rc != SQLITE_DONE) {
            std::cerr << "Error during sqlite3_step: " << sqlite3_errmsg(db) << std::endl;
        }

        // Finalize the statement to free up memory
        sqlite3_finalize(stmt);
    } else {
        // Log the SQLite error message if prepare fails
        std::cerr << "Failed to retrieve tags: " << sqlite3_errmsg(db) << std::endl;
    }

    return tags;
}

bool Model::removeTagFromModel(int modelId, const std::string& tagName) {
    // Get the tag ID first
    std::string sqlTagId = "SELECT id FROM tags WHERE name = ?;";
    sqlite3_stmt* stmt;
    int tagId = -1;

    if (sqlite3_prepare_v2(db, sqlTagId.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, tagName.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            tagId = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    if (tagId != -1) {
        // Delete the association in the model_tags table
        std::string sql = "DELETE FROM model_tags WHERE model_id = ? AND tag_id = ?;";
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, modelId);
            sqlite3_bind_int(stmt, 2, tagId);
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                std::cerr << "Failed to delete tag from model: " << sqlite3_errmsg(db) << std::endl;
                sqlite3_finalize(stmt);
                return false;
            }
            sqlite3_finalize(stmt);
            return true;
        }
    }

    std::cerr << "Tag not found or error occurred: " << sqlite3_errmsg(db) << std::endl;
    return false;
}

bool Model::executeSQL(const std::string& sql) {
  char* errMsg = nullptr;
  int rc = sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg);
  if (rc != SQLITE_OK) {
    std::cerr << "SQL error: " << errMsg << std::endl;
    sqlite3_free(errMsg);
    return false;
  }
  return true;
}

int Model::hashModel(const std::string& modelDir) {
  // std::cout << "--Trying to open file: " << modelDir << std::endl;
  // if (!std::filesystem::exists(modelDir)) {
  //   std::cerr << "File does not exist: " << modelDir << std::endl;
  //   return 0;
  // }
  std::ifstream file(modelDir);
  if (!file.is_open()) {
    // throw std::runtime_error("Could not open file");
    std::cerr << "Could not open file" << std::endl;
    return 0;
  }

  std::stringstream buffer;
  buffer << file.rdbuf(); 
  std::string fileContents = buffer.str();

  std::hash<std::string> hasher;
  return hasher(fileContents);
}