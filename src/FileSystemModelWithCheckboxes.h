#ifndef FILESYSTEMMODELWITHCHECKBOXES_H
#define FILESYSTEMMODELWITHCHECKBOXES_H

#include <QFileSystemModel>
#include <QMap>
#include "Model.h"

class FileSystemModelWithCheckboxes : public QFileSystemModel {
    Q_OBJECT

public:
    explicit FileSystemModelWithCheckboxes(Model* model, const QString& rootPath, QObject* parent = nullptr);

    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

    bool isIncluded(const QModelIndex& index) const;
    void setIncluded(const QModelIndex& index, bool included);

signals:
    void inclusionChanged(const QModelIndex& index, bool included);

private:
    Model* model;
    QString rootPath;
    QModelIndex rootIndex;
    mutable QMap<QString, Qt::CheckState> m_checkStates;

    void updateChildren(const QModelIndex& index, Qt::CheckState state);
    void updateParent(const QModelIndex& index);
    void initializeCheckStates(const QModelIndex& parentIndex);
};

#endif // FILESYSTEMMODELWITHCHECKBOXES_H
