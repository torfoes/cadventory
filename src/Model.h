#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>
#include <sqlite3.h>


struct ModelData {
    int id;
    std::string short_name;
    std::string primary_file;
    std::string override_info;
};


class Model {
public:
  Model(const std::string& path);
  ~Model();

  bool createTable();

  // basic CRUD interface
  bool insertModel(const std::string& shortName, const std::string& file = "", const std::string& overrides = ""); // Create
  std::vector<ModelData> getModels(); // Read
  bool updateModel(int id, const std::string& shortName, const std::string& file = "", const std::string& overrides = ""); // Update
  bool deleteModel(int id); // Delete

private:
  sqlite3* db;
  std::string dbPath;

  bool executeSQL(const std::string& sql);
  static int callback(void* data, int argc, char** argv, char** azColName);
};


#endif // MODEL_H
