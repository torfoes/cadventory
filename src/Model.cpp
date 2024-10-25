// Model.cpp

#include "Model.h"
#include <QPixmap>
#include <QBuffer>
#include <QImageReader>
#include <QImageWriter>
#include <QVariant>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

Model::Model(const std::string& libraryPath, QObject* parent)
    : QAbstractListModel(parent), db(nullptr)
{
    // create a hidden directory inside the library path
    fs::path hiddenDir = fs::path(libraryPath) / ".cadventory";
    hiddenDirPath = hiddenDir.string();
    if (!fs::exists(hiddenDir)) {
        fs::create_directory(hiddenDir);
    }

    // set the database path inside the hidden directory
    dbPath = (hiddenDir / "metadata.db").string();

    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Can't open database at " << dbPath << ": " << sqlite3_errmsg(db) << std::endl;
    } else {
        std::cout << "Opened database at " << dbPath << " successfully" << std::endl;
        createTables();

        loadModelsFromDatabase();
    }
}

Model::~Model() {
    if (db) {
        sqlite3_close(db);
    }
}

bool Model::createTables() {
    std::string sqlModels = R"(
        CREATE TABLE IF NOT EXISTS models (
            id INTEGER PRIMARY KEY,
            short_name TEXT NOT NULL,
            primary_file TEXT,
            override_info TEXT,
            title TEXT,
            thumbnail BLOB,
            author TEXT,
            file_path TEXT,
            library_name TEXT,
            is_selected INTEGER DEFAULT 0
        );
    )";

    std::string sqlObjects = R"(
        CREATE TABLE IF NOT EXISTS objects (
            object_id INTEGER PRIMARY KEY AUTOINCREMENT,
            model_id INTEGER NOT NULL,
            name TEXT NOT NULL,
            parent_object_id INTEGER,
            is_selected INTEGER DEFAULT 0,
            FOREIGN KEY(model_id) REFERENCES models(id),
            FOREIGN KEY(parent_object_id) REFERENCES objects(object_id)
        );
    )";


    return executeSQL(sqlModels) && executeSQL(sqlObjects);
}

int Model::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return static_cast<int>(models.size());
}

QVariant Model::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(models.size()))
        return QVariant();

    const ModelData& modelData = models.at(static_cast<size_t>(index.row()));

    switch (role) {
    case Qt::DisplayRole:
    case ShortNameRole:
        return QString::fromStdString(modelData.short_name);
    case IdRole:
        return modelData.id;
    case PrimaryFileRole:
        return QString::fromStdString(modelData.primary_file);
    case OverrideInfoRole:
        return QString::fromStdString(modelData.override_info);
    case TitleRole:
        return QString::fromStdString(modelData.title);
    case ThumbnailRole:
        if (!modelData.thumbnail.empty()) {
            QPixmap thumbnail;
            thumbnail.loadFromData(reinterpret_cast<const uchar*>(modelData.thumbnail.data()),
                                   static_cast<uint>(modelData.thumbnail.size()), "PNG");
            return thumbnail;
        }
        return QVariant();
    case AuthorRole:
        return QString::fromStdString(modelData.author);
    case FilePathRole:
        return QString::fromStdString(modelData.file_path);
    case LibraryNameRole:
        return QString::fromStdString(modelData.library_name);
    case IsSelectedRole:
        return modelData.isSelected;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> Model::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[ShortNameRole] = "short_name";
    roles[PrimaryFileRole] = "primary_file";
    roles[OverrideInfoRole] = "override_info";
    roles[TitleRole] = "title";
    roles[ThumbnailRole] = "thumbnail";
    roles[AuthorRole] = "author";
    roles[FilePathRole] = "file_path";
    roles[LibraryNameRole] = "library_name";
    roles[IsSelectedRole] = "isSelected";
    return roles;
}

bool Model::insertModel(int id, const ModelData& modelData) {
    std::string sql = R"(
        INSERT OR IGNORE INTO models
        (id, short_name, primary_file, override_info, title, thumbnail, author, file_path, library_name, is_selected)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt;
    std::lock_guard<std::mutex> lock(db_mutex);

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);
        sqlite3_bind_text(stmt, 2, modelData.short_name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, modelData.primary_file.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, modelData.override_info.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, modelData.title.c_str(), -1, SQLITE_STATIC);

        if (!modelData.thumbnail.empty()) {
            sqlite3_bind_blob(stmt, 6, modelData.thumbnail.data(), static_cast<int>(modelData.thumbnail.size()), SQLITE_STATIC);
        } else {
            sqlite3_bind_null(stmt, 6);
        }

        sqlite3_bind_text(stmt, 7, modelData.author.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 8, modelData.file_path.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 9, modelData.library_name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 10, modelData.isSelected ? 1 : 0);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Insert model failed: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }
        sqlite3_finalize(stmt);

        // Update the models vector
        beginInsertRows(QModelIndex(), models.size(), models.size());
        models.push_back(modelData);
        endInsertRows();

        return true;
    } else {
        std::cerr << "SQL error in insertModel: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
}

bool Model::updateModel(int id, const ModelData& modelData) {
    std::string sql = R"(
        UPDATE models SET
            short_name = ?,
            primary_file = ?,
            override_info = ?,
            title = ?,
            thumbnail = ?,
            author = ?,
            file_path = ?,
            library_name = ?,
            is_selected = ?
        WHERE id = ?;
    )";

    sqlite3_stmt* stmt;
    std::lock_guard<std::mutex> lock(db_mutex);

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, modelData.short_name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, modelData.primary_file.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, modelData.override_info.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, modelData.title.c_str(), -1, SQLITE_STATIC);

        if (!modelData.thumbnail.empty()) {
            sqlite3_bind_blob(stmt, 5, modelData.thumbnail.data(), static_cast<int>(modelData.thumbnail.size()), SQLITE_STATIC);
        } else {
            sqlite3_bind_null(stmt, 5);
        }

        sqlite3_bind_text(stmt, 6, modelData.author.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 7, modelData.file_path.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 8, modelData.library_name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 9, modelData.isSelected ? 1 : 0);
        sqlite3_bind_int(stmt, 10, id);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Update model failed: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }
        sqlite3_finalize(stmt);

        // Update the models vector
        for (int row = 0; row < static_cast<int>(models.size()); ++row) {
            if (models[row].id == id) {
                models[row] = modelData;
                QModelIndex modelIndex = index(row);
                emit dataChanged(modelIndex, modelIndex);
                break;
            }
        }

        return true;
    } else {
        std::cerr << "SQL error in updateModel: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
}

bool Model::deleteModel(int id) {
    // First, delete associated objects
    if (!deleteObjectsForModel(id)) {
        return false;
    }

    std::string sql = "DELETE FROM models WHERE id = ?;";
    sqlite3_stmt* stmt;
    std::lock_guard<std::mutex> lock(db_mutex);

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Delete model failed: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }
        sqlite3_finalize(stmt);

        // Remove from models vector
        for (int row = 0; row < static_cast<int>(models.size()); ++row) {
            if (models[row].id == id) {
                beginRemoveRows(QModelIndex(), row, row);
                models.erase(models.begin() + row);
                endRemoveRows();
                break;
            }
        }

        return true;
    } else {
        std::cerr << "SQL error in deleteModel: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
}

bool Model::modelExists(int id) {
    std::string sql = "SELECT COUNT(*) FROM models WHERE id = ?;";
    sqlite3_stmt* stmt;
    int count = 0;
    std::lock_guard<std::mutex> lock(db_mutex);

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    } else {
        std::cerr << "SQL error in modelExists: " << sqlite3_errmsg(db) << std::endl;
    }

    return count > 0;
}

ModelData Model::getModelById(int id) {
    std::string sql = R"(
        SELECT id, short_name, primary_file, override_info, title, thumbnail, author, file_path, library_name, is_selected
        FROM models WHERE id = ?;
    )";
    sqlite3_stmt* stmt;
    ModelData model;
    std::lock_guard<std::mutex> lock(db_mutex);

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            model.id = sqlite3_column_int(stmt, 0);
            model.short_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            model.primary_file = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            model.override_info = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            model.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

            const void* blob = sqlite3_column_blob(stmt, 5);
            int blob_size = sqlite3_column_bytes(stmt, 5);
            if (blob && blob_size > 0) {
                model.thumbnail.assign(static_cast<const char*>(blob), static_cast<const char*>(blob) + blob_size);
            }

            model.author = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            model.file_path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            model.library_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
            model.isSelected = sqlite3_column_int(stmt, 9) != 0;
        }
        sqlite3_finalize(stmt);
    } else {
        std::cerr << "Failed to select model: " << sqlite3_errmsg(db) << std::endl;
    }

    return model;
}

void Model::loadModelsFromDatabase() {
    std::vector<ModelData> loadedModels;
    std::string sql = R"(
        SELECT id, short_name, primary_file, override_info, title, thumbnail, author, file_path, library_name, is_selected
        FROM models;
    )";
    sqlite3_stmt* stmt;
    std::lock_guard<std::mutex> lock(db_mutex);

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            ModelData model;
            model.id = sqlite3_column_int(stmt, 0);
            model.short_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            model.primary_file = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            model.override_info = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            model.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

            const void* blob = sqlite3_column_blob(stmt, 5);
            int blob_size = sqlite3_column_bytes(stmt, 5);
            if (blob && blob_size > 0) {
                model.thumbnail.assign(static_cast<const char*>(blob), static_cast<const char*>(blob) + blob_size);
            }

            model.author = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            model.file_path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            model.library_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
            model.isSelected = sqlite3_column_int(stmt, 9) != 0;

            loadedModels.push_back(model);
        }
        sqlite3_finalize(stmt);

        // Update the models vector
        beginResetModel();
        models = std::move(loadedModels);
        endResetModel();

    } else {
        std::cerr << "Failed to select models: " << sqlite3_errmsg(db) << std::endl;
    }
}

int Model::hashModel(const std::string& modelDir) {
    std::ifstream file(modelDir, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Could not open file for hashing: " << modelDir << std::endl;
        return 0;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string fileContents = buffer.str();

    std::hash<std::string> hasher;
    return static_cast<int>(hasher(fileContents));
}

std::string Model::getHiddenDirectoryPath() const {
    return hiddenDirPath;
}

bool Model::executeSQL(const std::string& sql) {
    char* errMsg = nullptr;
    std::lock_guard<std::mutex> lock(db_mutex);
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error in executeSQL: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

void Model::refreshModelData() {
    loadModelsFromDatabase();
}

bool Model::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(models.size()))
        return false;

    ModelData& modelData = models[index.row()];

    if (role == IsSelectedRole) {
        modelData.isSelected = value.toBool();
        emit dataChanged(index, index, {IsSelectedRole});
        return true;
    }

    return false;
}

Qt::ItemFlags Model::flags(const QModelIndex& index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}


int Model::insertObject(const ObjectData& obj) {
    std::string sql = R"(
        INSERT INTO objects (model_id, name, parent_object_id, is_selected)
        VALUES (?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt;
    std::lock_guard<std::mutex> lock(db_mutex);

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL error in insertObject: " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }

    sqlite3_bind_int(stmt, 1, obj.model_id);
    sqlite3_bind_text(stmt, 2, obj.name.c_str(), -1, SQLITE_STATIC);

    if (obj.parent_object_id != -1) {
        sqlite3_bind_int(stmt, 3, obj.parent_object_id);
    } else {
        sqlite3_bind_null(stmt, 3);
    }

    sqlite3_bind_int(stmt, 4, obj.is_selected ? 1 : 0);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Insert object failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return -1;
    }

    int object_id = static_cast<int>(sqlite3_last_insert_rowid(db));
    sqlite3_finalize(stmt);
    return object_id;
}


bool Model::deleteObjectsForModel(int model_id) {
    std::string sql = "DELETE FROM objects WHERE model_id = ?;";
    sqlite3_stmt* stmt;
    std::lock_guard<std::mutex> lock(db_mutex);

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, model_id);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Delete objects failed: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }
        sqlite3_finalize(stmt);
        return true;
    } else {
        std::cerr << "SQL error in deleteObjectsForModel: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
}


std::vector<ObjectData> Model::getObjectsForModel(int model_id) {
    std::vector<ObjectData> objects;
    std::string sql = R"(
        SELECT object_id, model_id, name, parent_object_id, is_selected
        FROM objects
        WHERE model_id = ?;
    )";

    sqlite3_stmt* stmt;
    std::lock_guard<std::mutex> lock(db_mutex);

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, model_id);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            ObjectData obj;
            obj.object_id = sqlite3_column_int(stmt, 0);
            obj.model_id = sqlite3_column_int(stmt, 1);
            obj.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

            if (sqlite3_column_type(stmt, 3) != SQLITE_NULL) {
                obj.parent_object_id = sqlite3_column_int(stmt, 3);
            } else {
                obj.parent_object_id = -1;
            }

            obj.is_selected = sqlite3_column_int(stmt, 4) != 0;
            objects.push_back(obj);
        }
        sqlite3_finalize(stmt);
    } else {
        std::cerr << "Failed to retrieve objects: " << sqlite3_errmsg(db) << std::endl;
    }

    return objects;
}


bool Model::setObjectData(int object_id, const QVariant& value, int role) {
    if (role == IsSelectedRole) {
        bool is_selected = value.toBool();
        return updateObjectSelection(object_id, is_selected);
    }
    return false;
}

bool Model::updateObjectSelection(int object_id, bool is_selected) {
    std::string sql = "UPDATE objects SET is_selected = ? WHERE object_id = ?;";

    sqlite3_stmt* stmt;
    std::lock_guard<std::mutex> lock(db_mutex);

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL error in updateObjectSelection: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, is_selected ? 1 : 0);
    sqlite3_bind_int(stmt, 2, object_id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Update object selection failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}


bool Model::updateObject(const ObjectData& obj) {
    std::string sql = R"(
        UPDATE objects SET
            name = ?,
            parent_object_id = ?,
            is_selected = ?
        WHERE object_id = ?;
    )";

    sqlite3_stmt* stmt;
    std::lock_guard<std::mutex> lock(db_mutex);

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL error in updateObject: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, obj.name.c_str(), -1, SQLITE_STATIC);
    if (obj.parent_object_id != -1) {
        sqlite3_bind_int(stmt, 2, obj.parent_object_id);
    } else {
        sqlite3_bind_null(stmt, 2);
    }
    sqlite3_bind_int(stmt, 3, obj.is_selected ? 1 : 0);
    sqlite3_bind_int(stmt, 4, obj.object_id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Update object failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

