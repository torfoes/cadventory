#include "GeometryBrowserDialog.h"
#include "Model.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QStyle>
#include <QTreeWidgetItem>
#include <QDebug>

#include <iostream>
#include <array>
#include <regex>
#include <sstream>

GeometryBrowserDialog::GeometryBrowserDialog(int modelId, Model* model, QWidget* parent)
    : QDialog(parent), modelId(modelId), model(model) {
    setWindowTitle("Geometry Browser");

    // load objects for the given model
    loadObjects();

    // Create UI components
    treeWidget = new QTreeWidget();
    treeWidget->setHeaderLabel("Objects");
    treeWidget->header()->setSectionResizeMode(QHeaderView::Stretch);
    treeWidget->setExpandsOnDoubleClick(false);
    treeWidget->setAnimated(true);

    // Populate the tree widget
    populateTreeWidget();

    // Layout
    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->addWidget(treeWidget);

    setLayout(mainLayout);

    // Connections
    connect(treeWidget, &QTreeWidget::itemChanged, this, &GeometryBrowserDialog::onItemChanged);
}

void GeometryBrowserDialog::loadObjects() {
    ModelData modelData = model->getModelById(modelId);

    // get the objects from the model
    std::vector<ObjectData> objects = model->getObjectsForModel(modelId);

    // Build the geometryData map and objectNameToIdMap
    for (const auto& obj : objects) {
        objectNameToIdMap[obj.name] = obj.object_id;

        if (obj.parent_object_id == -1) {
            // top-level object
            geometryData[obj.name] = std::vector<std::string>();
        } else {
            std::string parentName;
            for (const auto& potentialParent : objects) {
                if (potentialParent.object_id == obj.parent_object_id) {
                    parentName = potentialParent.name;
                    break;
                }
            }
            if (!parentName.empty()) {
                geometryData[parentName].push_back(obj.name);
            } else {
                // Handle missing parent (should not happen)
                geometryData[obj.name] = std::vector<std::string>();
            }
        }
    }
}

void GeometryBrowserDialog::populateTreeWidget() {
    QIcon folderClosedIcon = QApplication::style()->standardIcon(QStyle::SP_DirClosedIcon);
    QIcon fileIcon = QApplication::style()->standardIcon(QStyle::SP_FileIcon);

    // Map object names to their selection state
    std::map<std::string, bool> objectSelectionMap;
    for (const auto& [name, id] : objectNameToIdMap) {
        ObjectData objData = model->getObjectById(id);
        objectSelectionMap[name] = objData.is_selected;
    }

    // Create a mapping from object name to QTreeWidgetItem
    std::map<std::string, QTreeWidgetItem*> itemMap;

    // First pass: create all items
    for (const auto& [name, _] : objectNameToIdMap) {
        QTreeWidgetItem* item = new QTreeWidgetItem();
        item->setText(0, QString::fromStdString(name));
        item->setCheckState(0, objectSelectionMap[name] ? Qt::Checked : Qt::Unchecked);

        itemMap[name] = item;
    }

    // Second pass: set up hierarchy
    for (const auto& [parentName, childrenNames] : geometryData) {
        QTreeWidgetItem* parentItem = itemMap[parentName];
        for (const auto& childName : childrenNames) {
            QTreeWidgetItem* childItem = itemMap[childName];
            parentItem->addChild(childItem);
        }
    }

    // Add top-level items to the tree widget
    for (const auto& [name, item] : itemMap) {
        if (geometryData.find(name) != geometryData.end()) {
            treeWidget->addTopLevelItem(item);
            item->setIcon(0, folderClosedIcon);
        } else {
            // This is a leaf node without children
            if (!item->parent()) {
                treeWidget->addTopLevelItem(item);
            }
            item->setIcon(0, fileIcon);
        }
    }

    // Expand all items
    treeWidget->expandAll();
}

void GeometryBrowserDialog::onItemChanged(QTreeWidgetItem* item, int column) {
    Q_UNUSED(column);
    QString itemName = item->text(0);
    std::string objectName = itemName.toStdString();

    if (objectNameToIdMap.find(objectName) != objectNameToIdMap.end()) {
        int objectId = objectNameToIdMap[objectName];
        bool isSelected = (item->checkState(0) == Qt::Checked);

        // Update the selection state using the Model
        model->setObjectData(objectId, isSelected, Model::IsSelectedRole);

        // After updating the selection, you might want to perform additional actions
        // For example, regenerate the thumbnail or update other UI elements
    }
}
