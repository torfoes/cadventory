#include "FileSystemModelWithCheckboxes.h"

#include <QDir>
#include <QDebug>

FileSystemModelWithCheckboxes::FileSystemModelWithCheckboxes(Model* model, const QString& rootPath, QObject* parent)
    : QFileSystemModel(parent), model(model), rootPath(QDir::cleanPath(rootPath))
{
    // Set filters to display directories and files
    setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);

    // Connect the directoryLoaded signal to initializeCheckStates slot
    connect(this, &QFileSystemModel::directoryLoaded, this, &FileSystemModelWithCheckboxes::onDirectoryLoaded);

    // Set the root path and store the root index
    rootIndex = setRootPath(this->rootPath);
}

FileSystemModelWithCheckboxes::~FileSystemModelWithCheckboxes()
{
}

void FileSystemModelWithCheckboxes::initializeCheckStates(const QModelIndex& parentIndex)
{
    qDebug() << "initializeCheckStates";

    int rowCount = this->rowCount(parentIndex);
    qDebug() << rowCount;

    for (int i = 0; i < rowCount; ++i)
    {
        QModelIndex index = this->index(i, 0, parentIndex);
        QString path = filePath(index);

        qDebug() << "Processing path:" << path;

        if (isDir(index))
        {
            // Recursively initialize child items
            initializeCheckStates(index);
        }
        else
        {
            // For files (e.g., .g files)
            if (path.endsWith(".g", Qt::CaseInsensitive))
            {
                qDebug() << "Found .g file:" << path;

                std::string filePathStd = QDir::cleanPath(path).toStdString();
                ModelData modelData = model->getModelByFilePath(filePathStd);

                qDebug() << "getModelByFilePath returned id:" << modelData.id;

                if (modelData.id == 0)
                {
                    // Model not found in database, create a new one
                    modelData.short_name = QFileInfo(path).fileName().toStdString();
                    modelData.file_path = filePathStd;
                    modelData.is_included = true; // Mark as included
                    modelData.is_processed = false; // Not processed

                    qDebug() << "Inserting new model for file:" << path;

                    bool insertSuccess = model->insertModel(modelData);

                    qDebug() << "Insert model success:" << insertSuccess;

                    // Fetch the inserted model to get the assigned id
                    modelData = model->getModelByFilePath(filePathStd);
                    qDebug() << "After insertion, model id is:" << modelData.id;
                }

                // Update checkStates based on is_included
                Qt::CheckState state = modelData.is_included ? Qt::Checked : Qt::Unchecked;
                {
                    QMutexLocker locker(&m_checkStatesMutex);
                    m_checkStates[path] = state;
                }
            }
            else
            {
                qDebug() << "Skipping non-.g file:" << path;
            }
        }
    }
}

QVariant FileSystemModelWithCheckboxes::data(const QModelIndex& index, int role) const
{
    if (role == Qt::CheckStateRole && index.column() == 0)
    {
        QString path = filePath(index);

        // For directories, determine check state based on children
        if (isDir(index))
        {
            // Return a tri-state checkbox
            int checkedCount = 0;
            int uncheckedCount = 0;

            int rowCount = this->rowCount(index);
            for (int i = 0; i < rowCount; ++i)
            {
                QModelIndex childIndex = this->index(i, 0, index);
                QVariant childData = data(childIndex, Qt::CheckStateRole);
                Qt::CheckState childState = static_cast<Qt::CheckState>(childData.toInt());
                if (childState == Qt::Checked)
                    ++checkedCount;
                else if (childState == Qt::Unchecked)
                    ++uncheckedCount;
            }

            if (checkedCount == rowCount)
                return Qt::Checked;
            else if (uncheckedCount == rowCount)
                return Qt::Unchecked;
            else
                return Qt::PartiallyChecked;
        }
        else
        {
            QMutexLocker locker(&m_checkStatesMutex);
            if (m_checkStates.contains(path))
                return m_checkStates[path];
            else
                return Qt::Unchecked;
        }
    }

    return QFileSystemModel::data(index, role);
}

bool FileSystemModelWithCheckboxes::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role == Qt::CheckStateRole && index.column() == 0)
    {
        QString path = filePath(index);
        Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
        bool included = (state == Qt::Checked);

        if (isDir(index))
        {
            // Update all children
            updateChildren(index, state);
        }
        else
        {
            std::string filePathStd = path.toStdString();
            ModelData modelData = model->getModelByFilePath(filePathStd);

            if (modelData.id != 0)
            {
                modelData.is_included = included;
                model->updateModel(modelData.id, modelData);
            }
            else if (included)
            {
                // Model not found, create a new one
                modelData.short_name = QFileInfo(path).fileName().toStdString();
                modelData.file_path = filePathStd;
                modelData.is_included = true;
                modelData.is_processed = false;
                model->insertModel(modelData);

                // Fetch the inserted model to get the assigned id
                modelData = model->getModelByFilePath(filePathStd);
            }

            {
                QMutexLocker locker(&m_checkStatesMutex);
                m_checkStates[path] = state;
            }

            emit dataChanged(index, index, {Qt::CheckStateRole});
            emit inclusionChanged(index, included);
        }

        // Update parent check state
        updateParent(index);

        return true;
    }

    return QFileSystemModel::setData(index, value, role);
}

Qt::ItemFlags FileSystemModelWithCheckboxes::flags(const QModelIndex& index) const
{
    return QFileSystemModel::flags(index) | Qt::ItemIsUserCheckable;
}

void FileSystemModelWithCheckboxes::updateChildren(const QModelIndex& index, Qt::CheckState state)
{
    int rowCount = this->rowCount(index);
    for (int i = 0; i < rowCount; ++i)
    {
        QModelIndex childIndex = this->index(i, 0, index);
        QString path = filePath(childIndex);

        if (isDir(childIndex))
        {
            updateChildren(childIndex, state);
        }
        else
        {
            if (path.endsWith(".g", Qt::CaseInsensitive))
            {
                bool included = (state == Qt::Checked);
                std::string filePathStd = path.toStdString();
                ModelData modelData = model->getModelByFilePath(filePathStd);

                if (modelData.id != 0)
                {
                    modelData.is_included = included;
                    model->updateModel(modelData.id, modelData);
                }
                else if (included)
                {
                    // Model not found, create a new one
                    modelData.short_name = QFileInfo(path).fileName().toStdString();
                    modelData.file_path = filePathStd;
                    modelData.is_included = true;
                    modelData.is_processed = false;
                    model->insertModel(modelData);

                    // Fetch the inserted model to get the assigned id
                    modelData = model->getModelByFilePath(filePathStd);
                }

                {
                    QMutexLocker locker(&m_checkStatesMutex);
                    m_checkStates[path] = state;
                }

                emit dataChanged(childIndex, childIndex, {Qt::CheckStateRole});
                emit inclusionChanged(childIndex, included);
            }
        }
    }
}

void FileSystemModelWithCheckboxes::updateParent(const QModelIndex& index)
{
    if (!index.isValid())
        return;

    QModelIndex parentIndex = index.parent();
    if (!parentIndex.isValid())
        return;

    int rowCount = this->rowCount(parentIndex);
    int checkedCount = 0;
    int uncheckedCount = 0;

    for (int i = 0; i < rowCount; ++i)
    {
        QModelIndex siblingIndex = this->index(i, 0, parentIndex);
        QVariant siblingData = data(siblingIndex, Qt::CheckStateRole);
        Qt::CheckState siblingState = static_cast<Qt::CheckState>(siblingData.toInt());
        if (siblingState == Qt::Checked)
            ++checkedCount;
        else if (siblingState == Qt::Unchecked)
            ++uncheckedCount;
    }

    Qt::CheckState parentState;
    if (checkedCount == rowCount)
        parentState = Qt::Checked;
    else if (uncheckedCount == rowCount)
        parentState = Qt::Unchecked;
    else
        parentState = Qt::PartiallyChecked;

    {
        QMutexLocker locker(&m_checkStatesMutex);
        m_checkStates[filePath(parentIndex)] = parentState;
    }

    emit dataChanged(parentIndex, parentIndex, {Qt::CheckStateRole});

    // Recursively update parent
    updateParent(parentIndex);
}

void FileSystemModelWithCheckboxes::refresh()
{
    // Clear the check states
    {
        QMutexLocker locker(&m_checkStatesMutex);
        m_checkStates.clear();
    }

    // Reset the model
    beginResetModel();

    // Optionally, reset internal data structures or caches
    // For example, you might clear any custom data you've stored

    // Reinitialize the model
    setRootPath(rootPath);
    rootIndex = index(rootPath);

    // Reinitialize check states starting from the root index
    initializeCheckStates(rootIndex);

    endResetModel();
}

void FileSystemModelWithCheckboxes::onDirectoryLoaded(const QString& path)
{
    qDebug() << "Directory loaded in FileSystemModelWithCheckboxes:" << path;
    QModelIndex index = this->index(path);
    initializeCheckStates(index);
}
