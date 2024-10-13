#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>
#include <map>
#include <sqlite3.h>


struct ModelData {
    int id;
    std::string short_name;
    std::string primary_file;
    std::string override_info;
    std::map<std::string, std::string> properties;
};


class Model {
public:
  Model(const std::string& path);
  ~Model();

  bool createTable();

  // model CRUD interface
  bool insertModel(int id, const std::string& shortName, const std::string& file = "", const std::string& overrides = ""); // Create
  bool insertModel(const std::string& filePath, const std::string& shortName, const std::string& primaryFile, const std::string& overrides);
  std::vector<ModelData> getModels();
  ModelData getModelById(int id);
  bool updateModel(int id, const std::string& shortName, const std::string& file = "", const std::string& overrides = ""); // Update
  bool deleteModel(int id);

  // tag CRUD + association interface
  bool addTagToModel(int modelId, const std::string& tagName);
  std::vector<std::string> getTagsForModel(int modelId);
  bool removeTagFromModel(int modelId, const std::string& tagName);

  // properties
  bool insertProperty(int modelId, const std::string& key, const std::string& value);
  bool insertProperties(int modelId, const std::map<std::string, std::string>& properties);
  std::string getProperty(int modelId, const std::string& key); 
  std::map<std::string, std::string> getProperties(int modelId);
  bool updateProperty(int modelId, const std::string& key, const std::string& value);
  bool deleteProperty(int modelId, const std::string& key);
  
  int hashModel(const std::string& modelDir);
  std::string dbPath;
private:
  sqlite3* db;

  bool executeSQL(const std::string& sql);
  static int callback(void* data, int argc, char** argv, char** azColName);
  
};


#endif // MODEL_H
