#include "Model.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

// Constructor and Destructor
Model::Model(const std::string& path) : db(nullptr), dbPath(path) {
  if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
    std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
  } else {
    std::cout << "Opened database successfully" << std::endl;
    createTable();
  }
}

Model::~Model() {
  if (db) {
    sqlite3_close(db);
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

sqlite3_stmt* Model::prepareStatement(const std::string& sql) {
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db)
              << std::endl;
    return nullptr;
  }
  return stmt;
}

bool Model::executePreparedStatement(sqlite3_stmt* stmt) {
  if (sqlite3_step(stmt) != SQLITE_DONE) {
    std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
    sqlite3_finalize(stmt);
    return false;
  }
  sqlite3_finalize(stmt);
  return true;
}

ModelData Model::mapRowToModelData(sqlite3_stmt* stmt) {
  return {sqlite3_column_int(stmt, 0),
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)),
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)),
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)),
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5))};
}

bool Model::createTable() {
  std::string sqlModels = R"(
        CREATE TABLE IF NOT EXISTS models (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            short_name TEXT NOT NULL,
            path TEXT NOT NULL,
            primary_file_path TEXT,
            library TEXT NOT NULL
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

// CRUD Operations for Models
bool Model::insertModel(const std::string& filePath,
                        const std::string& shortName, const std::string& path,
                        const std::string& primaryFile,
                        const std::string& overrides,
                        const std::string& library) {
  int id = hashModel(filePath);
  return insertModel(id, shortName, path, primaryFile, overrides, library);
}

bool Model::insertModel(int id, const std::string& shortName,
                        const std::string& path, const std::string& primaryFile,
                        const std::string& overrides,
                        const std::string& library) {
  std::string sql =
      "INSERT INTO models (id, short_name, path, primary_file_path, "
      "override_info, library) "
      "VALUES (?, ?, ?, ?, ?, ?);";

  sqlite3_stmt* stmt = prepareStatement(sql);
  if (!stmt) return false;

  sqlite3_bind_int(stmt, 1, id);
  sqlite3_bind_text(stmt, 2, shortName.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 3, path.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 4, primaryFile.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 5, overrides.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 6, library.c_str(), -1, SQLITE_STATIC);

  return executePreparedStatement(stmt);
}

// gets the models from the database
std::vector<ModelData> Model::getModels() {
  std::vector<ModelData> models;
  std::string sql =
      "SELECT id, short_name, path, primary_file_path, override_info, library "
      "FROM "
      "models;";
  sqlite3_stmt* stmt = prepareStatement(sql);
  if (!stmt) return models;

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    models.push_back(mapRowToModelData(stmt));
  }
  sqlite3_finalize(stmt);
  return models;
}

ModelData Model::getModelById(int id) {
  std::string sql =
      "SELECT id, short_name, path, primary_file_path, override_info, library "
      "FROM "
      "models WHERE id = ?;";
  sqlite3_stmt* stmt = prepareStatement(sql);
  if (!stmt) return {0, "", "", "", "", ""};

  sqlite3_bind_int(stmt, 1, id);
  ModelData model = {0, "", "", "", "", ""};

  if (sqlite3_step(stmt) == SQLITE_ROW) {
    model = mapRowToModelData(stmt);
  }
  sqlite3_finalize(stmt);
  return model;
}

bool Model::updateModel(int id, const std::string& shortName,
                        const std::string& path, const std::string& primaryFile,
                        const std::string& overrides,
                        const std::string& library) {
  std::string sql =
      "UPDATE models SET short_name = ?, path = ?, primary_file_path = ?, "
      "override_info = ?, library = ? "
      "WHERE id = ?;";
  sqlite3_stmt* stmt = prepareStatement(sql);
  if (!stmt) return false;

  sqlite3_bind_text(stmt, 1, shortName.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 2, path.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 3, primaryFile.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 4, overrides.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 5, library.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_int(stmt, 6, id);

  return executePreparedStatement(stmt);
}

bool Model::deleteModel(int id) {
  std::string sql = "DELETE FROM models WHERE id = ?;";
  sqlite3_stmt* stmt = prepareStatement(sql);
  if (!stmt) return false;

  sqlite3_bind_int(stmt, 1, id);

  return executePreparedStatement(stmt);
}

// Tag Operations
bool Model::addTagToModel(int modelId, const std::string& tagName) {
  // Insert the tag if it doesn't already exist
  std::string sqlTagInsert = "INSERT OR IGNORE INTO tags (name) VALUES (?);";
  sqlite3_stmt* stmt = prepareStatement(sqlTagInsert);
  if (!stmt) return false;

  sqlite3_bind_text(stmt, 1, tagName.c_str(), -1, SQLITE_STATIC);
  if (!executePreparedStatement(stmt)) return false;

  // Get tag ID
  int tagId = getTagId(tagName);
  if (tagId == -1) return false;

  // Link the tag to the model
  std::string sqlLink =
      "INSERT INTO model_tags (model_id, tag_id) VALUES (?, ?);";
  stmt = prepareStatement(sqlLink);
  if (!stmt) return false;

  sqlite3_bind_int(stmt, 1, modelId);
  sqlite3_bind_int(stmt, 2, tagId);

  return executePreparedStatement(stmt);
}

int Model::getTagId(const std::string& tagName) {
  std::string sqlTagId = "SELECT id FROM tags WHERE name = ?;";
  sqlite3_stmt* stmt = prepareStatement(sqlTagId);
  if (!stmt) return -1;

  sqlite3_bind_text(stmt, 1, tagName.c_str(), -1, SQLITE_STATIC);
  int tagId = -1;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    tagId = sqlite3_column_int(stmt, 0);
  }
  sqlite3_finalize(stmt);
  return tagId;
}

std::vector<std::string> Model::getTagsForModel(int modelId) {
  std::vector<std::string> tags;
  std::string sql =
      "SELECT name FROM tags t JOIN model_tags mt ON t.id = mt.tag_id WHERE "
      "mt.model_id = ?;";
  sqlite3_stmt* stmt = prepareStatement(sql);
  if (!stmt) return tags;

  sqlite3_bind_int(stmt, 1, modelId);

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    const char* tagText =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    if (tagText) {
      tags.push_back(tagText);
    }
  }
  sqlite3_finalize(stmt);
  return tags;
}

bool Model::removeTagFromModel(int modelId, const std::string& tagName) {
  int tagId = getTagId(tagName);
  if (tagId == -1) return false;

  // Delete the association in the model_tags table
  std::string sql = "DELETE FROM model_tags WHERE model_id = ? AND tag_id = ?;";
  sqlite3_stmt* stmt = prepareStatement(sql);
  if (!stmt) return false;

  sqlite3_bind_int(stmt, 1, modelId);
  sqlite3_bind_int(stmt, 2, tagId);

  return executePreparedStatement(stmt);
}

// Property Operations
bool Model::insertProperty(int modelId, const std::string& key,
                           const std::string& value) {
  // Insert or replace the property
  std::string sql =
      "INSERT OR REPLACE INTO model_properties (model_id, property_key, "
      "property_value) "
      "VALUES (?, ?, ?);";
  sqlite3_stmt* stmt = prepareStatement(sql);
  if (!stmt) return false;

  sqlite3_bind_int(stmt, 1, modelId);
  sqlite3_bind_text(stmt, 2, key.c_str(), -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 3, value.c_str(), -1, SQLITE_STATIC);

  return executePreparedStatement(stmt);
}

bool Model::insertProperties(
    int modelId, const std::map<std::string, std::string>& properties) {
  for (const auto& [key, value] : properties) {
    if (!insertProperty(modelId, key, value)) {
      return false;
    }
  }
  return true;
}

std::string Model::getProperty(int modelId, const std::string& key) {
  std::string sql =
      "SELECT property_value FROM model_properties WHERE model_id = ? AND "
      "property_key = ?;";
  sqlite3_stmt* stmt = prepareStatement(sql);
  if (!stmt) return "";

  sqlite3_bind_int(stmt, 1, modelId);
  sqlite3_bind_text(stmt, 2, key.c_str(), -1, SQLITE_STATIC);

  std::string value;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    value = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
  }
  sqlite3_finalize(stmt);
  return value;
}

std::map<std::string, std::string> Model::getProperties(int modelId) {
  std::map<std::string, std::string> properties;
  std::string sql =
      "SELECT property_key, property_value FROM model_properties WHERE "
      "model_id = ?;";
  sqlite3_stmt* stmt = prepareStatement(sql);
  if (!stmt) return properties;

  sqlite3_bind_int(stmt, 1, modelId);

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    std::string key =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    std::string value =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    properties[key] = value;
  }
  sqlite3_finalize(stmt);
  return properties;
}

bool Model::updateProperty(int modelId, const std::string& key,
                           const std::string& value) {
  return insertProperty(modelId, key, value);
}

bool Model::deleteProperty(int modelId, const std::string& key) {
  std::string sql =
      "DELETE FROM model_properties WHERE model_id = ? AND property_key = ?;";
  sqlite3_stmt* stmt = prepareStatement(sql);
  if (!stmt) return false;

  sqlite3_bind_int(stmt, 1, modelId);
  sqlite3_bind_text(stmt, 2, key.c_str(), -1, SQLITE_STATIC);

  return executePreparedStatement(stmt);
}

bool Model::hasProperties(int modelId) {
  std::string sql =
      "SELECT 1 FROM model_properties WHERE model_id = ? LIMIT 1;";
  sqlite3_stmt* stmt = prepareStatement(sql);
  if (!stmt) return false;

  sqlite3_bind_int(stmt, 1, modelId);
  bool hasProperties = (sqlite3_step(stmt) == SQLITE_ROW);
  sqlite3_finalize(stmt);
  return hasProperties;
}

// Utility Methods
void Model::printModel(int modelId) {
  ModelData model = getModelById(modelId);
  std::cout << "Model ID: " << model.id << std::endl;
  std::cout << "Short Name: " << model.short_name << std::endl;
  std::cout << "Path: " << model.path << std::endl;
  std::cout << "Primary File: " << model.primary_file_path << std::endl;
  std::cout << "Library: " << model.library << std::endl;
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
