#include "FileSystemModelWithCheckboxes.h"
#include <QDir>
#include <QDebug>

FileSystemModelWithCheckboxes::FileSystemModelWithCheckboxes(Model* model, const QString& rootPath, QObject* parent)
    : QFileSystemModel(parent), model(model), rootPath(QDir::cleanPath(rootPath))
{
    setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);
}

Qt::ItemFlags FileSystemModelWithCheckboxes::flags(const QModelIndex& index) const {
    Qt::ItemFlags flags = QFileSystemModel::flags(index);
    if (index.isValid() && index.column() == 0) {
        flags |= Qt::ItemIsUserCheckable;
    }
    return flags;
}

QVariant FileSystemModelWithCheckboxes::data(const QModelIndex& index, int role) const {
    if (role == Qt::CheckStateRole && index.column() == 0) {
        QString path = filePath(index);
        return m_checkStates.value(path, Qt::Unchecked);
    }
    return QFileSystemModel::data(index, role);
}

bool FileSystemModelWithCheckboxes::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (role == Qt::CheckStateRole && index.column() == 0) {
        QString path = filePath(index);
        Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
        m_checkStates[path] = state;

        // Update inclusion in the model
        std::string filePathStd = path.toStdString();
        ModelData modelData = model->getModelByFilePath(filePathStd);
        bool included = (state == Qt::Checked);

        if (modelData.id != 0) {
            // Update existing model
            modelData.is_included = included;
            model->updateModel(modelData.id, modelData);
        } else if (included) {
            // Insert new model with is_included = true
            int id = model->hashModel(filePathStd);
            modelData.id = id;
            modelData.file_path = filePathStd;
            modelData.is_included = true;
            modelData.is_processed = false;
            model->insertModel(id, modelData);
        }

        // Update children and parent
        updateChildren(index, state);
        updateParent(index);

        emit dataChanged(index, index, {Qt::CheckStateRole});
        emit inclusionChanged(index, included);

        return true;
    }
    return QFileSystemModel::setData(index, value, role);
}

void FileSystemModelWithCheckboxes::updateChildren(const QModelIndex& index, Qt::CheckState state) {
    int rowCount = this->rowCount(index);
    for (int i = 0; i < rowCount; ++i) {
        QModelIndex childIndex = this->index(i, 0, index);
        QString path = filePath(childIndex);
        m_checkStates[path] = state;

        // Update inclusion in the model
        std::string filePathStd = path.toStdString();
        ModelData modelData = model->getModelByFilePath(filePathStd);
        bool included = (state == Qt::Checked);

        if (modelData.id != 0) {
            modelData.is_included = included;
            model->updateModel(modelData.id, modelData);
        } else if (included) {
            int id = model->hashModel(filePathStd);
            modelData.id = id;
            modelData.file_path = filePathStd;
            modelData.is_included = true;
            modelData.is_processed = false;
            model->insertModel(id, modelData);
        }

        emit dataChanged(childIndex, childIndex, {Qt::CheckStateRole});
        updateChildren(childIndex, state);
    }
}

void FileSystemModelWithCheckboxes::updateParent(const QModelIndex& index) {
    QModelIndex parentIndex = index.parent();
    if (!parentIndex.isValid())
        return;

    int checkedCount = 0;
    int uncheckedCount = 0;

    int rowCount = this->rowCount(parentIndex);
    for (int i = 0; i < rowCount; ++i) {
        QModelIndex siblingIndex = this->index(i, 0, parentIndex);
        QString path = filePath(siblingIndex);
        Qt::CheckState state = m_checkStates.value(path, Qt::Unchecked);
        if (state == Qt::Checked)
            ++checkedCount;
        else if (state == Qt::Unchecked)
            ++uncheckedCount;
    }

    QString parentPath = filePath(parentIndex);
    if (checkedCount == rowCount) {
        m_checkStates[parentPath] = Qt::Checked;
    } else if (uncheckedCount == rowCount) {
        m_checkStates[parentPath] = Qt::Unchecked;
    } else {
        m_checkStates[parentPath] = Qt::PartiallyChecked;
    }

    emit dataChanged(parentIndex, parentIndex, {Qt::CheckStateRole});
    updateParent(parentIndex);
}
