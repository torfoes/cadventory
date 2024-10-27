#ifndef GEOMETRYBROWSERDIALOG_H
#define GEOMETRYBROWSERDIALOG_H

#include <QDialog>
#include <QTreeWidget>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <map>
#include <vector>
#include <string>

class Model;

class GeometryBrowserDialog : public QDialog {
    Q_OBJECT
public:
    explicit GeometryBrowserDialog(int modelId, Model* model, QWidget* parent = nullptr);

private slots:
    void onItemChanged(QTreeWidgetItem* item, int column);

private:
    void loadObjects();
    void populateTreeWidget();

    int modelId;
    Model* model;

    QTreeWidget* treeWidget;

    std::map<std::string, std::vector<std::string>> geometryData;
    std::map<std::string, int> objectNameToIdMap;
};

#endif // GEOMETRYBROWSERDIALOG_H
