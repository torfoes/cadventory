#ifndef MODEL_H
#define MODEL_H

#include <sqlite3.h>

#include <QAbstractListModel>
#include <mutex>
#include <string>
#include <vector>

// ModelData structure
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
  bool is_selected;
  bool is_processed;
  std::vector<std::string> tags;
};

// Declare ModelData as a Qt metatype
Q_DECLARE_METATYPE(ModelData)

// ObjectData structure
struct ObjectData {
  int object_id;
  int model_id;
  std::string name;
  int parent_object_id;
  bool is_selected;
};

class Model : public QAbstractListModel {
  Q_OBJECT

 public:
  enum ModelRoles {
    IdRole = Qt::UserRole + 1,
    ShortNameRole,
    PrimaryFileRole,
    OverrideInfoRole,
    TitleRole,
    ThumbnailRole,
    AuthorRole,
    FilePathRole,
    LibraryNameRole,
    IsSelectedRole
  };

  explicit Model(const std::string& libraryPath, QObject* parent = nullptr);
  ~Model();

  // override QAbstractListModel methods
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index,
                int role = Qt::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const override;

  // added methods for data modification and item flags
  bool setData(const QModelIndex& index, const QVariant& value,
               int role = Qt::EditRole) override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;

    // CRUD operations for models
    bool insertModel(int id, const ModelData& modelData);
    bool updateModel(int id, const ModelData& modelData);
    bool deleteModel(int id);
    bool modelExists(int id);
    bool deleteTables();
    void resetDatabase();

  // Getters
  ModelData getModelById(int id);

  // Utility methods
  int hashModel(const std::string& modelDir);
  void refreshModelData();
  void printModel(const ModelData& modelData);

  std::string getHiddenDirectoryPath() const;

  // Update the model list from the database
  void loadModelsFromDatabase();

  // Methods for objects
  int insertObject(const ObjectData& obj);
  bool updateObject(const ObjectData& obj);
  bool deleteObjectsForModel(int model_id);
  std::vector<ObjectData> getObjectsForModel(int model_id);
  bool setObjectData(int object_id, const QVariant& value, int role);
  bool updateObjectSelection(int object_id, bool is_selected);
  ObjectData getObjectById(int object_id);

  // Retrieve all selected models
  std::vector<ModelData> getSelectedModels();

  // Retrieve selected objects for a given model ID
  std::vector<ObjectData> getSelectedObjectsForModel(int model_id);

    void beginTransaction();
    void commitTransaction();
    bool updateObjectParentId(int object_id, int parent_object_id);


  // Tag operations
  bool addTagToModel(int modelId, const std::string& tagName);
  int getTagId(const std::string& tagName);
  std::vector<std::string> getAllTags();
  std::vector<std::string> getTagsForModel(int modelId);
  bool removeTagFromModel(int modelId, const std::string& tagName);
  bool removeAllTagsFromModel(int modelId);

  // Properties operations
  bool setPropertyForModel(int modelId, const std::string& key,
                           const std::string& value);
  std::map<std::string, std::string> getPropertiesForModel(int model_id);

  // Simplifying executions
  sqlite3_stmt* prepareStatement(const std::string& sql);
  bool executePreparedStatement(sqlite3_stmt* stmt);

 private:
  // Database related
  bool createTables();
  bool executeSQL(const std::string& sql);
  sqlite3* db;
  std::string dbPath;
  std::mutex db_mutex;

  std::string hiddenDirPath;

  std::vector<ModelData> models;
};

#endif  // MODEL_H
