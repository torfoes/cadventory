// FileSystemFilterProxyModel.cpp

#include "FileSystemFilterProxyModel.h"
#include <QFileSystemModel>
#include <QFileInfo>

FileSystemFilterProxyModel::FileSystemFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    setRecursiveFilteringEnabled(true);
}

bool FileSystemFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    QFileSystemModel* fsModel = static_cast<QFileSystemModel*>(sourceModel());
    if (!fsModel)
        return false;

    QFileInfo fileInfo = fsModel->fileInfo(index);

    if (fileInfo.isDir()) {
        // Accept the directory only if it has accepted children (contains .g files in its subtree)
        return hasAcceptedChildren(index);
    } else {
        // Accept the file only if it has a .g extension
        return fileInfo.suffix().compare("g", Qt::CaseInsensitive) == 0;
    }
}

bool FileSystemFilterProxyModel::hasAcceptedChildren(const QModelIndex& sourceIndex, int depth) const
{
    const int maxDepth = 1000; // Set an appropriate maximum depth
    if (depth > maxDepth)
        return false;

    QFileSystemModel* fsModel = static_cast<QFileSystemModel*>(sourceModel());
    if (!fsModel)
        return false;

    int rowCount = fsModel->rowCount(sourceIndex);
    for (int row = 0; row < rowCount; ++row) {
        QModelIndex childIndex = fsModel->index(row, 0, sourceIndex);
        QFileInfo fileInfo = fsModel->fileInfo(childIndex);

        if (fileInfo.isDir()) {
            // Recursively check if the directory has accepted children
            if (hasAcceptedChildren(childIndex, depth + 1))
                return true;
        } else {
            // Accept the file only if it has a .g extension
            if (fileInfo.suffix().compare("g", Qt::CaseInsensitive) == 0)
                return true;
        }
    }
    return false;
}
