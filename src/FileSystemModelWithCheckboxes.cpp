#include "FileSystemModelWithCheckboxes.h"
#include <QDir>
#include <QDebug>

FileSystemModelWithCheckboxes::FileSystemModelWithCheckboxes(Model* model, const QString& rootPath, QObject* parent)
    : QFileSystemModel(parent),
    model(model),
    rootPath(QDir::cleanPath(rootPath))
{
    // Set filters to display directories and files
    setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);

    // Set the root path and store the root index
    rootIndex = setRootPath(this->rootPath);

    // Initialize check states starting from the root index
    initializeCheckStates(rootIndex);
}

Qt::ItemFlags FileSystemModelWithCheckboxes::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags = QFileSystemModel::flags(index);
    if (index.isValid() && index.column() == 0) {
        flags |= Qt::ItemIsUserCheckable;
    }
    return flags;
}

QVariant FileSystemModelWithCheckboxes::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::CheckStateRole && index.column() == 0) {
        QString path = filePath(index);

        // Return the stored check state
        return m_checkStates.value(path, Qt::Unchecked);
    }

    return QFileSystemModel::data(index, role);
}

bool FileSystemModelWithCheckboxes::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid())
        return false;

    if (role == Qt::CheckStateRole && index.column() == 0) {
        QString path = filePath(index);
        Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
        m_checkStates[path] = state;

        // Update children recursively
        updateChildren(index, state);

        // Update parent recursively
        updateParent(index);

        emit dataChanged(index, index, {Qt::CheckStateRole});
        emit inclusionChanged(index, state == Qt::Checked);

        // Update the model's inclusion state
        bool included = (state == Qt::Checked);
        std::string filePathStd = path.toStdString();
        ModelData modelData = model->getModelByFilePath(filePathStd);

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

        return true;
    }
    return QFileSystemModel::setData(index, value, role);
}

bool FileSystemModelWithCheckboxes::isIncluded(const QModelIndex& index) const
{
    QString path = filePath(index);
    return m_checkStates.value(path, Qt::Unchecked) == Qt::Checked;
}

void FileSystemModelWithCheckboxes::setIncluded(const QModelIndex& index, bool included)
{
    setData(index, included ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
}

void FileSystemModelWithCheckboxes::updateChildren(const QModelIndex& index, Qt::CheckState state)
{
    int rowCount = this->rowCount(index);
    for (int i = 0; i < rowCount; ++i) {
        QModelIndex childIndex = this->index(i, 0, index);
        QString path = filePath(childIndex);
        m_checkStates[path] = state;

        emit dataChanged(childIndex, childIndex, {Qt::CheckStateRole});

        // Recursive call
        updateChildren(childIndex, state);
    }
}

void FileSystemModelWithCheckboxes::updateParent(const QModelIndex& index)
{
    QModelIndex parentIndex = index.parent();
    if (!parentIndex.isValid() || parentIndex == rootIndex)
        return;

    int rowCount = this->rowCount(parentIndex);
    int checkedCount = 0;
    int uncheckedCount = 0;

    for (int i = 0; i < rowCount; ++i) {
        QModelIndex siblingIndex = this->index(i, 0, parentIndex);
        Qt::CheckState siblingState = m_checkStates.value(filePath(siblingIndex), Qt::Unchecked);
        if (siblingState == Qt::Checked)
            checkedCount++;
        else if (siblingState == Qt::Unchecked)
            uncheckedCount++;
    }

    Qt::CheckState parentState = Qt::Unchecked;
    if (checkedCount == rowCount) {
        parentState = Qt::Checked;
    } else if (uncheckedCount == rowCount) {
        parentState = Qt::Unchecked;
    } else {
        parentState = Qt::PartiallyChecked;
    }

    QString path = filePath(parentIndex);
    m_checkStates[path] = parentState;

    emit dataChanged(parentIndex, parentIndex, {Qt::CheckStateRole});

    // Recursive call
    updateParent(parentIndex);
}

void FileSystemModelWithCheckboxes::initializeCheckStates(const QModelIndex& parentIndex)
{
    if (!parentIndex.isValid())
        return;

    int rowCount = this->rowCount(parentIndex);
    for (int i = 0; i < rowCount; ++i) {
        QModelIndex index = this->index(i, 0, parentIndex);
        QString path = filePath(index);
        std::string filePathStd = path.toStdString();
        bool included = model->isFileIncluded(filePathStd);

        // Set initial check state
        m_checkStates[path] = included ? Qt::Checked : Qt::Unchecked;

        // Recursive call
        initializeCheckStates(index);
    }

    // Update parent states
    if (parentIndex != rootIndex) {
        updateParent(parentIndex);
    }
}
