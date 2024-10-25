#ifndef MODEL_H
#define MODEL_H

#include <QAbstractListModel>
#include <vector>
#include <string>
#include <mutex>
#include <sqlite3.h>

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
    bool isSelected;
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
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // added methods for data modification and item flags
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // CRUD operations for models
    bool insertModel(int id, const ModelData& modelData);
    bool updateModel(int id, const ModelData& modelData);
    bool deleteModel(int id);
    bool modelExists(int id);

    // Getters
    ModelData getModelById(int id);

    // Utility methods
    int hashModel(const std::string& modelDir);
    void refreshModelData();

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

#endif // MODEL_H
