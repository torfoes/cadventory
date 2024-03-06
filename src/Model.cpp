
#include "Model.h"
#include <iostream>


Model::Model(const std::string& path) : dbPath(path), db(nullptr) {
  if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
    std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
  } else {
    // init on instantiation
    createTable();
  }
}


Model::~Model() {
  if (db) {
    sqlite3_close(db);
  }
}


bool
Model::createTable() {
  std::string sql = R"(
        CREATE TABLE IF NOT EXISTS models (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            short_name TEXT NOT NULL UNIQUE,
            primary_file TEXT,
            override_info TEXT
);
)";

  return executeSQL(sql);
}


bool
Model::insertModel(const std::string& shortName, const std::string& primaryFile, const std::string& overrides) {
  std::string sql = "INSERT INTO models (short_name, primary_file, override_info) VALUES (?, ?, ?);";

  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, shortName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, primaryFile.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, overrides.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
      std::cerr << "Insert model failed: " << sqlite3_errmsg(db) << std::endl;
      sqlite3_finalize(stmt);
      return false;
    }
    sqlite3_finalize(stmt);
    return true;
  } else {
    std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
  }

  return false;
}


std::vector<ModelData>
Model::getModels() {
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


bool
Model::updateModel(int id, const std::string& shortName, const std::string& primaryFile, const std::string& overrides) {
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


bool
Model::executeSQL(const std::string& sql) {
  char* errMsg = nullptr;
  int rc = sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg);
  if (rc != SQLITE_OK) {
    std::cerr << "SQL error: " << errMsg << std::endl;
    sqlite3_free(errMsg);
    return false;
  }
  return true;
}
