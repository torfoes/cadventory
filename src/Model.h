// Model.h

#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>
#include <mutex>
#include <sqlite3.h>

// structure to hold model data
struct ModelData {
    int id;
    std::string short_name;
    std::string primary_file;
    std::string override_info;
    std::string title;
    std::vector<char> thumbnail;
    std::string author;
    std::string file_path;
    std::string library_name;
};

class Model {
public:
    Model(const std::string& libraryPath);
    ~Model();

    // Model CRUD operations
    bool insertModel(int id, const ModelData& modelData);
    bool updateModel(int id, const ModelData& modelData);
    bool deleteModel(int id);
    bool modelExists(int id);

    // Getters
    std::vector<ModelData> getModels();
    ModelData getModelById(int id);

    // Utility methods
    int hashModel(const std::string& modelDir);
    void printModel(int modelId);

    // get the path to the hidden directory
    std::string getHiddenDirectoryPath() const;

private:
    // Database related
    bool createTables();
    bool executeSQL(const std::string& sql);
    sqlite3* db;
    std::string dbPath;
    std::mutex db_mutex;

    // Hidden directory path
    std::string hiddenDirPath;
};

#endif // MODEL_H
