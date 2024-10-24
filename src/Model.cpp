#include "Model.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

Model::Model(const std::string& path) : db(nullptr), dbPath(path) {
  if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
    std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
  } else {
    // Initialize tables upon instantiation
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
            path TEXT NOT NULL,
            primary_file_path TEXT,
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
  std::string sqlModelProperties = R"(
        CREATE TABLE IF NOT EXISTS model_properties (
            model_id INTEGER NOT NULL,
            property_key TEXT NOT NULL,
            property_value TEXT,
            PRIMARY KEY (model_id, property_key),
            FOREIGN KEY (model_id) REFERENCES models(id) ON DELETE CASCADE
        );
    )";

  return executeSQL(sqlModels) && executeSQL(sqlTags) &&
         executeSQL(sqlModelTags) && executeSQL(sqlModelProperties);
}

bool Model::insertModel(const std::string& filePath,
                        const std::string& shortName, const std::string& path,
                        const std::string& primaryFile,
                        const std::string& overrides) {
  int id = hashModel(filePath);
  return insertModel(id, shortName, path, primaryFile, overrides);
}

bool Model::insertModel(int id, const std::string& shortName,
                        const std::string& path, const std::string& primaryFile,
                        const std::string& overrides) {
  std::string sql =
      "INSERT INTO models (id, short_name, path, primary_file_path, "
      "override_info) "
      "VALUES (?, ?, ?, ?, ?);";

  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_int(stmt, 1, id);
    sqlite3_bind_text(stmt, 2, shortName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, path.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, primaryFile.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, overrides.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
      sqlite3_finalize(stmt);
      return false;
    }
    sqlite3_finalize(stmt);
    return true;
  } else {
    std::cout << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    return false;
  }
}

std::vector<ModelData> Model::getModels() {
  std::vector<ModelData> models;
  std::string sql =
      "SELECT id, short_name, path, primary_file_path, override_info FROM "
      "models;";
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      models.push_back(
          {sqlite3_column_int(stmt, 0),
           reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
           reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)),
           reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)),
           reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4))});
    }
    sqlite3_finalize(stmt);
  } else {
    std::cerr << "Failed to select models: " << sqlite3_errmsg(db) << std::endl;
  }
  return models;
}

ModelData Model::getModelById(int id) {
  std::string sql =
      "SELECT id, short_name, path, primary_file_path, override_info FROM "
      "models WHERE id = ?;";
  sqlite3_stmt* stmt;
  ModelData model = {0, "", "", "", ""};

  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_int(stmt, 1, id);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      model = {sqlite3_column_int(stmt, 0),
               reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
               reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)),
               reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)),
               reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4))};
    }
    sqlite3_finalize(stmt);
  } else {
    std::cerr << "Failed to select model: " << sqlite3_errmsg(db) << std::endl;
  }

  return model;
}

bool Model::updateModel(int id, const std::string& shortName,
                        const std::string& path, const std::string& primaryFile,
                        const std::string& overrides) {
  std::string sql =
      "UPDATE models SET short_name = ?, path = ?, primary_file_path = ?, "
      "override_info = ? "
      "WHERE id = ?;";
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, shortName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, path.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, primaryFile.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, overrides.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, id);

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

  if (sqlite3_prepare_v2(db, sqlTagInsert.c_str(), -1, &stmt, nullptr) ==
      SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, tagName.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
  } else {
    std::cerr << "Failed to insert tag: " << sqlite3_errmsg(db) << std::endl;
    return false;
  }

  // Link the tag to the model
  std::string sqlTagId = "SELECT id FROM tags WHERE name = ?;";
  if (sqlite3_prepare_v2(db, sqlTagId.c_str(), -1, &stmt, nullptr) ==
      SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, tagName.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      int tagId = sqlite3_column_int(stmt, 0);
      sqlite3_finalize(stmt);

      std::string sqlLink =
          "INSERT INTO model_tags (model_id, tag_id) VALUES (?, ?);";
      if (sqlite3_prepare_v2(db, sqlLink.c_str(), -1, &stmt, nullptr) ==
          SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, modelId);
        sqlite3_bind_int(stmt, 2, tagId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return true;
      }
    }
    sqlite3_finalize(stmt);
  }

  std::cerr << "Failed to add tag to model: " << sqlite3_errmsg(db)
            << std::endl;
  return false;
}

std::vector<std::string> Model::getTagsForModel(int modelId) {
  std::vector<std::string> tags;
  std::string sql =
      "SELECT name FROM tags t JOIN model_tags mt ON t.id = mt.tag_id WHERE "
      "mt.model_id = ?;";
  sqlite3_stmt* stmt = nullptr;
  int rc;

  rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
  if (rc == SQLITE_OK) {
    sqlite3_bind_int(stmt, 1, modelId);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
      const char* tagText =
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));

      if (tagText) {
        tags.push_back(tagText);
      } else {
        std::cerr << "Null value encountered for tag name." << std::endl;
      }
    }

    if (rc != SQLITE_DONE) {
      std::cerr << "Error during sqlite3_step: " << sqlite3_errmsg(db)
                << std::endl;
    }

    sqlite3_finalize(stmt);
  } else {
    std::cerr << "Failed to retrieve tags: " << sqlite3_errmsg(db) << std::endl;
  }

  return tags;
}

bool Model::removeTagFromModel(int modelId, const std::string& tagName) {
  // Get the tag ID first
  std::string sqlTagId = "SELECT id FROM tags WHERE name = ?;";
  sqlite3_stmt* stmt;
  int tagId = -1;

  if (sqlite3_prepare_v2(db, sqlTagId.c_str(), -1, &stmt, nullptr) ==
      SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, tagName.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      tagId = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
  }

  if (tagId != -1) {
    // Delete the association in the model_tags table
    std::string sql =
        "DELETE FROM model_tags WHERE model_id = ? AND tag_id = ?;";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
      sqlite3_bind_int(stmt, 1, modelId);
      sqlite3_bind_int(stmt, 2, tagId);
      if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Failed to delete tag from model: " << sqlite3_errmsg(db)
                  << std::endl;
        sqlite3_finalize(stmt);
        return false;
      }
      sqlite3_finalize(stmt);
      return true;
    }
  }

  std::cerr << "Tag not found or error occurred: " << sqlite3_errmsg(db)
            << std::endl;
  return false;
}

bool Model::insertProperty(int modelId, const std::string& key,
                           const std::string& value) {
  // Check if the property already exists
  std::string checkSql =
      "SELECT COUNT(*) FROM model_properties WHERE model_id = ? AND "
      "property_key = ?;";
  sqlite3_stmt* checkStmt;
  if (sqlite3_prepare_v2(db, checkSql.c_str(), -1, &checkStmt, nullptr) ==
      SQLITE_OK) {
    sqlite3_bind_int(checkStmt, 1, modelId);
    sqlite3_bind_text(checkStmt, 2, key.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(checkStmt) == SQLITE_ROW &&
        sqlite3_column_int(checkStmt, 0) > 0) {
      sqlite3_finalize(checkStmt);
      return false;  // Property already exists
    }
    sqlite3_finalize(checkStmt);
  } else {
    std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    return false;
  }

  // Insert the new property
  std::string sql =
      "INSERT INTO model_properties (model_id, property_key, property_value) "
      "VALUES (?, ?, ?);";
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_int(stmt, 1, modelId);
    sqlite3_bind_text(stmt, 2, key.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, value.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
      std::cerr << "Failed to insert property: " << sqlite3_errmsg(db)
                << std::endl;
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

bool Model::insertProperties(
    int modelId, const std::map<std::string, std::string>& properties) {
  std::string sql =
      "INSERT INTO model_properties (model_id, property_key, property_value) "
      "VALUES (?, ?, ?);";
  sqlite3_stmt* stmt;

  for (const auto& [key, value] : properties) {
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
      sqlite3_bind_int(stmt, 1, modelId);
      sqlite3_bind_text(stmt, 2, key.c_str(), -1, SQLITE_STATIC);
      sqlite3_bind_text(stmt, 3, value.c_str(), -1, SQLITE_STATIC);

      if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Failed to insert property: " << sqlite3_errmsg(db)
                  << std::endl;
        sqlite3_finalize(stmt);
        return false;
      }
      sqlite3_finalize(stmt);
    } else {
      std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
      return false;
    }
  }
  return true;
}

std::string Model::getProperty(int modelId, const std::string& key) {
  std::string sql =
      "SELECT property_value FROM model_properties WHERE model_id = ? AND "
      "property_key = ?;";
  sqlite3_stmt* stmt;
  std::string value;

  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_int(stmt, 1, modelId);
    sqlite3_bind_text(stmt, 2, key.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
      value = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    }
    sqlite3_finalize(stmt);
  } else {
    std::cerr << "Failed to fetch property: " << sqlite3_errmsg(db)
              << std::endl;
  }

  return value;
}

std::map<std::string, std::string> Model::getProperties(int modelId) {
  std::map<std::string, std::string> properties;
  std::string sql =
      "SELECT property_key, property_value FROM model_properties WHERE "
      "model_id = ?;";
  sqlite3_stmt* stmt;

  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_int(stmt, 1, modelId);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      std::string key =
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
      std::string value =
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
      properties[key] = value;
    }
    sqlite3_finalize(stmt);
  } else {
    std::cerr << "Failed to fetch properties for model: " << sqlite3_errmsg(db)
              << std::endl;
  }
  return properties;
}

bool Model::updateProperty(int modelId, const std::string& key,
                           const std::string& value) {
  std::string sql =
      "UPDATE model_properties SET property_value = ? WHERE model_id = ? AND "
      "property_key = ?;";
  sqlite3_stmt* stmt;

  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, value.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, modelId);
    sqlite3_bind_text(stmt, 3, key.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
      std::cerr << "Failed to update property: " << sqlite3_errmsg(db)
                << std::endl;
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

bool Model::deleteProperty(int modelId, const std::string& key) {
  std::string sql =
      "DELETE FROM model_properties WHERE model_id = ? AND property_key = ?;";
  sqlite3_stmt* stmt;

  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_int(stmt, 1, modelId);
    sqlite3_bind_text(stmt, 2, key.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
      std::cerr << "Failed to delete property: " << sqlite3_errmsg(db)
                << std::endl;
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

bool Model::hasProperties(int modelId) {
  std::string sql =
      "SELECT 1 FROM model_properties WHERE model_id = ? LIMIT 1;";
  sqlite3_stmt* stmt;
  bool hasProperties = false;

  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_int(stmt, 1, modelId);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
      hasProperties = true;
    }
    sqlite3_finalize(stmt);
  } else {
    std::cerr << "Failed to check properties: " << sqlite3_errmsg(db)
              << std::endl;
  }

  return hasProperties;
}

void Model::printModel(int modelId) {
  ModelData model = getModelById(modelId);
  std::cout << "Model ID: " << model.id << std::endl;
  std::cout << "Short Name: " << model.short_name << std::endl;
  std::cout << "Path: " << model.path << std::endl;
  std::cout << "Primary File: " << model.primary_file_path << std::endl;
  std::cout << "Override Info: " << model.override_info << std::endl;

  std::map<std::string, std::string> properties = getProperties(modelId);
  std::cout << "Properties:" << std::endl;
  for (const auto& [key, value] : properties) {
    std::cout << "  " << key << ": " << value << std::endl;
  }

  std::vector<std::string> tags = getTagsForModel(modelId);
  std::cout << "Tags:" << std::endl;
  for (const std::string& tag : tags) {
    std::cout << "  " << tag << std::endl;
  }
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
  std::ifstream file(modelDir);
  if (!file.is_open()) {
    std::cerr << "Could not open file: " << modelDir << std::endl;
    return 0;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string fileContents = buffer.str();

  std::hash<std::string> hasher;
  return hasher(fileContents);
}
