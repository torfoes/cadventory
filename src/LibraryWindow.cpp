// LibraryWindow.cpp

#include "LibraryWindow.h"
#include "IndexingWorker.h"
#include "ProcessGFiles.h"
#include "MainWindow.h"
#include "GeometryBrowserDialog.h"
// #include "AdvancedOptionsDialog.h"

#include <QThread>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListView>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>


#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

LibraryWindow::LibraryWindow(QWidget* parent)
    : QWidget(parent),
    library(nullptr),
    mainWindow(nullptr),
    model(nullptr),
    availableModelsProxyModel(new ModelFilterProxyModel(this)),
    selectedModelsProxyModel(new ModelFilterProxyModel(this)),
    indexingThread(nullptr),
    indexingWorker(nullptr),

    modelCardDelegate(new ModelCardDelegate(this)) {
    ui.setupUi(this);

    // Set up models and views
    setupModelsAndViews();

    // Set up connections
    setupConnections();
}

LibraryWindow::~LibraryWindow() {
    qDebug() << "LibraryWindow destructor called";

    // Ensure the indexing thread is stopped if it wasn't already
    if (indexingThread && indexingThread->isRunning()) {
        qDebug() << "Waiting for indexingThread to finish in destructor";
        indexingThread->wait();
        qDebug() << "indexingThread finished in destructor";
    }

    // Delete indexingWorker and indexingThread if they exist
    if (indexingWorker) {
        delete indexingWorker;
        indexingWorker = nullptr;
        qDebug() << "indexingWorker deleted in destructor";
    }

    if (indexingThread) {
        delete indexingThread;
        indexingThread = nullptr;
        qDebug() << "indexingThread deleted in destructor";
    }
}


void LibraryWindow::loadFromLibrary(Library* _library) {
    library = _library;
    setWindowTitle(library->name() + QString(" Library"));
    ui.currentLibrary->setText(library->name());

    // Load models from the library
    model = library->model;

    // Set the source model for proxy models
    availableModelsProxyModel->setSourceModel(model);
    selectedModelsProxyModel->setSourceModel(model);

    // Set initial filters
    availableModelsProxyModel->setFilterRole(Model::IsSelectedRole);
    availableModelsProxyModel->setFilterFixedString("0"); // Show unselected models

    selectedModelsProxyModel->setFilterRole(Model::IsSelectedRole);
    selectedModelsProxyModel->setFilterFixedString("1"); // Show selected models

    startIndexing();
}

void LibraryWindow::startIndexing() {
    // Create the indexing worker and thread
    indexingThread = new QThread(this); // Parent is LibraryWindow
    indexingWorker = new IndexingWorker(library);

    // Move the worker to the thread
    indexingWorker->moveToThread(indexingThread);

    // Connect signals and slots
    connect(indexingThread, &QThread::started, indexingWorker, &IndexingWorker::process);
    connect(indexingWorker, &IndexingWorker::modelProcessed, this, &LibraryWindow::onModelProcessed);
    connect(indexingWorker, &IndexingWorker::progressUpdated, this, &LibraryWindow::onProgressUpdated);


    // Start the indexing thread
    indexingThread->start();
}



void LibraryWindow::setMainWindow(MainWindow* mainWindow) {
    this->mainWindow = mainWindow;
}

void LibraryWindow::setupModelsAndViews() {
    // Configure available models view
    ui.availableModelsView->setModel(availableModelsProxyModel);
    ui.availableModelsView->setItemDelegate(modelCardDelegate);
    ui.availableModelsView->setViewMode(QListView::ListMode);
    ui.availableModelsView->setFlow(QListView::TopToBottom);
    ui.availableModelsView->setWrapping(false);
    ui.availableModelsView->setResizeMode(QListView::Adjust);
    ui.availableModelsView->setSpacing(0);
    ui.availableModelsView->setUniformItemSizes(true);
    ui.availableModelsView->setSelectionMode(QAbstractItemView::NoSelection);
    ui.availableModelsView->setSelectionBehavior(QAbstractItemView::SelectRows);

    QSize itemSize = modelCardDelegate->sizeHint(QStyleOptionViewItem(), QModelIndex());
    ui.availableModelsView->setGridSize(QSize(0, itemSize.height()));

    ui.selectedModelsView->setModel(selectedModelsProxyModel);
    ui.selectedModelsView->setItemDelegate(modelCardDelegate);
    ui.selectedModelsView->setViewMode(QListView::ListMode);
    ui.selectedModelsView->setFlow(QListView::TopToBottom);
    ui.selectedModelsView->setWrapping(false);
    ui.selectedModelsView->setResizeMode(QListView::Adjust);
    ui.selectedModelsView->setSpacing(0);
    ui.selectedModelsView->setUniformItemSizes(true);
    ui.selectedModelsView->setSelectionMode(QAbstractItemView::NoSelection);
    ui.selectedModelsView->setSelectionBehavior(QAbstractItemView::SelectRows);

    // set grid size for selected models view
    ui.selectedModelsView->setGridSize(QSize(1, itemSize.height()));

    // Populate search field combo box
    // Add other fields as needed
}


void LibraryWindow::setupConnections() {
    // Connect search input
    connect(ui.searchLineEdit, &QLineEdit::textChanged, this, &LibraryWindow::onSearchTextChanged);
    connect(ui.searchFieldComboBox, &QComboBox::currentTextChanged, this, &LibraryWindow::onSearchFieldChanged);

    // Connect clicks on available models
    connect(ui.availableModelsView, &QListView::clicked, this, &LibraryWindow::onAvailableModelClicked);

    // Connect clicks on selected models
    connect(ui.selectedModelsView, &QListView::clicked, this, &LibraryWindow::onSelectedModelClicked);

    // Connect Generate Report button
    connect(ui.generateReportButton, &QPushButton::clicked, this, &LibraryWindow::onGenerateReportButtonClicked);

    // Connect settings clicked signal from delegate
    // connect(modelCardDelegate, &ModelCardDelegate::settingsClicked, this, &LibraryWindow::onSettingsClicked);
    // connect(ui.backButton, &QPushButton::clicked, this, &LibraryWindow::onBackButtonClicked);

    connect(modelCardDelegate, &ModelCardDelegate::geometryBrowserClicked,
            this, &LibraryWindow::onGeometryBrowserClicked);

}

void LibraryWindow::onSearchTextChanged(const QString& text) {
    int role = ui.searchFieldComboBox->currentData().toInt();
    availableModelsProxyModel->setFilterRole(role);
    availableModelsProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    availableModelsProxyModel->setFilterFixedString(text);
}

void LibraryWindow::onSearchFieldChanged(const QString& field) {
    Q_UNUSED(field);
    // Update filter role based on selected field
    int role = ui.searchFieldComboBox->currentData().toInt();
    availableModelsProxyModel->setFilterRole(role);
    // Re-apply filter
    availableModelsProxyModel->invalidate();
}

void LibraryWindow::onAvailableModelClicked(const QModelIndex& index) {
    // Toggle selection state
    QModelIndex sourceIndex = availableModelsProxyModel->mapToSource(index);
    bool isSelected = model->data(sourceIndex, Model::IsSelectedRole).toBool();
    model->setData(sourceIndex, !isSelected, Model::IsSelectedRole);

    // Update filters
    availableModelsProxyModel->invalidate();
    selectedModelsProxyModel->invalidate();
}

void LibraryWindow::onSelectedModelClicked(const QModelIndex& index) {
    // Toggle selection state
    QModelIndex sourceIndex = selectedModelsProxyModel->mapToSource(index);
    bool isSelected = model->data(sourceIndex, Model::IsSelectedRole).toBool();
    model->setData(sourceIndex, !isSelected, Model::IsSelectedRole);

    // Update filters
    availableModelsProxyModel->invalidate();
    selectedModelsProxyModel->invalidate();
}


void LibraryWindow::onGenerateReportButtonClicked() {
    std::vector<ModelData> selectedModels = model->getSelectedModels();

    if (selectedModels.empty()) {
        QMessageBox::information(this, "Report", "No models selected for the report.");
        return;
    }

    // Iterate through each selected model
    for (const auto& modelData : selectedModels) {
        std::cout << "==============================\n";
        std::cout << "Model ID: " << modelData.id << "\n";
        std::cout << "Short Name: " << modelData.short_name << "\n";
        std::cout << "Primary File: " << modelData.primary_file << "\n";
        std::cout << "Override Info: " << modelData.override_info << "\n";
        std::cout << "Title: " << modelData.title << "\n";
        std::cout << "Author: " << modelData.author << "\n";
        std::cout << "File Path: " << modelData.file_path << "\n";
        std::cout << "Library Name: " << modelData.library_name << "\n";
        std::cout << "Is Selected: " << (modelData.is_selected ? "Yes" : "No") << "\n";
        std::cout << "Is Processed: " << (modelData.is_processed ? "Yes" : "No") << "\n";

        // log thumbnail information
        if (!modelData.thumbnail.empty()) {
            std::cout << "Thumbnail Size: " << modelData.thumbnail.size() << " bytes\n";
        } else {
            std::cout << "Thumbnail: None\n";
        }

        // retrieve associated objects for the current model
        std::vector<ObjectData> associatedObjects = model->getObjectsForModel(modelData.id);

        if (associatedObjects.empty()) {
            std::cout << "No associated objects for this model.\n";
        } else {
            std::cout << "Associated Objects (" << associatedObjects.size() << "):\n";

            for (const auto& obj : associatedObjects) {
                std::cout << "  -------------------------\n";
                std::cout << "  Object ID: " << obj.object_id << "\n";
                std::cout << "  Name: " << obj.name << "\n";
                std::cout << "  Parent Object ID: "
                          << (obj.parent_object_id != -1 ? std::to_string(obj.parent_object_id) : "None")
                          << "\n";
                std::cout << "  Is Selected: " << (obj.is_selected ? "Yes" : "No") << "\n";
            }
            std::cout << "  -------------------------\n";
        }

        std::cout << "==============================\n\n";
    }

    QMessageBox::information(this, "Report", "Report has been logged to the console.");
}


void LibraryWindow::onSettingsClicked(int modelId) {
    // Handle settings button click
    qDebug() << "Settings button clicked for model ID:" << modelId;
    // Implement settings dialog or other actions here
}


void LibraryWindow::onModelProcessed(int modelId) {
    model->refreshModelData();
    availableModelsProxyModel->invalidate();
    selectedModelsProxyModel->invalidate();
}

void LibraryWindow::on_backButton_clicked() {
    qDebug() << "Back button clicked";

    if (indexingWorker) {
        qDebug() << "Requesting indexingWorker to stop";
        indexingWorker->stop();
        qDebug() << "indexingWorker->stop() called";
    } else {
        qDebug() << "indexingWorker is null or already deleted";
    }

    // Hide the LibraryWindow
    this->hide();
    qDebug() << "LibraryWindow hidden";

    // Show the MainWindow
    if (mainWindow) {
        mainWindow->show();
        qDebug() << "MainWindow shown";
    } else {
        qDebug() << "mainWindow is null";
    }

}


void LibraryWindow::onGeometryBrowserClicked(int modelId) {
    qDebug() << "Geometry browser clicked for model ID:" << modelId;

    GeometryBrowserDialog* dialog = new GeometryBrowserDialog(modelId, model, this);
    dialog->exec();
}

void LibraryWindow::onProgressUpdated(const QString& currentObject, int percentage) {
    ui.progressBar->setValue(percentage);

    if (percentage >= 100) {
        ui.statusLabel->setText("Processing complete");
        ui.progressBar->setVisible(false);
    } else {
        ui.statusLabel->setText(QString("Processing: %1").arg(currentObject));
    }
}
