#ifndef FILESYSTEMFILTERPROXYMODEL_H
#define FILESYSTEMFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class FileSystemFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit FileSystemFilterProxyModel(QObject* parent = nullptr);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
    bool hasAcceptedChildren(const QModelIndex& sourceIndex, int depth = 0) const;
};

#endif // FILESYSTEMFILTERPROXYMODEL_H
