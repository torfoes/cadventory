#ifndef FILESYSTEMMODELWITHCHECKBOXES_H
#define FILESYSTEMMODELWITHCHECKBOXES_H

#include <QFileSystemModel>
#include <QHash>
#include <QMutex>

#include "Model.h"

class FileSystemModelWithCheckboxes : public QFileSystemModel
{
    Q_OBJECT

public:
    explicit FileSystemModelWithCheckboxes(Model* model, const QString& rootPath, QObject* parent = nullptr);
    ~FileSystemModelWithCheckboxes() override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    void refresh();

public slots:
    void onDirectoryLoaded(const QString& path);

signals:
    void inclusionChanged(const QModelIndex& index, bool included);

protected:
    void initializeCheckStates(const QModelIndex& parentIndex);

private:
    Model* model;
    QString rootPath;
    QModelIndex rootIndex;

    mutable QHash<QString, Qt::CheckState> m_checkStates;
    mutable QMutex m_checkStatesMutex;

    void updateChildren(const QModelIndex& index, Qt::CheckState state);
    void updateParent(const QModelIndex& index);
};

#endif // FILESYSTEMMODELWITHCHECKBOXES_H
