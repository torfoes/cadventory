#ifndef MODEL_H
#define MODEL_H

#include <sqlite3.h>

#include <map>
#include <string>
#include <vector>

// Data structure representing a model record
struct ModelData {
  int id;
  std::string short_name;
  std::string path;
  std::string primary_file_path;
  std::string library;
  std::string override_info;
};

// Model class encapsulating database operations
class Model {
 public:
  // Constructor and Destructor
  Model(const std::string& path);
  ~Model();

  // Table creation
  bool createTable();

  // CRUD operations for models
  bool insertModel(ModelData modelData);
  std::vector<ModelData> getModels();
  std::vector<ModelData> getModelsInLibrary(const std::string& library);
  ModelData getModelById(int id);
  bool updateModel(int id, const std::string& shortName,
                   const std::string& path, const std::string& primaryFile,
                   const std::string& overrides, const std::string& library);
  bool deleteModel(int id);

  // Tag operations
  bool addTagToModel(int modelId, const std::string& tagName);
  std::vector<std::string> getTagsForModel(int modelId);
  bool removeTagFromModel(int modelId, const std::string& tagName);

  // Property operations
  bool insertProperty(int modelId, const std::string& key,
                      const std::string& value);
  bool insertProperties(int modelId,
                        const std::map<std::string, std::string>& properties);
  std::string getProperty(int modelId, const std::string& key);
  std::map<std::string, std::string> getProperties(int modelId);
  bool updateProperty(int modelId, const std::string& key,
                      const std::string& value);
  bool deleteProperty(int modelId, const std::string& key);
  bool hasProperties(int modelId);

  // Utility methods
  void printModel(int modelId);
  void printModel(ModelData model);
  int hashModel(const std::string& modelDir);

  // Database path
  std::string dbPath;

 private:
  sqlite3* db;

  // Utility methods for database operations
  bool executeSQL(const std::string& sql);
  sqlite3_stmt* prepareStatement(const std::string& sql);
  bool executePreparedStatement(sqlite3_stmt* stmt);
  ModelData mapRowToModelData(sqlite3_stmt* stmt);
  int getTagId(const std::string& tagName);
};

#endif  // MODEL_H
