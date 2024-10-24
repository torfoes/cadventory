// LibraryWindow.cpp

#include "LibraryWindow.h"
#include "IndexingWorker.h"
#include "ProcessGFiles.h"
#include "MainWindow.h"
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
    // clean up indexing thread
    if (indexingThread) {
        indexingThread->quit();
        indexingThread->wait();
        delete indexingWorker;
        delete indexingThread;
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
    indexingThread = new QThread(this);
    indexingWorker = new IndexingWorker(library);

    // Move the worker to the thread
    indexingWorker->moveToThread(indexingThread);

    // Connect signals and slots
    connect(indexingThread, &QThread::started, indexingWorker, &IndexingWorker::process);
    connect(indexingWorker, &IndexingWorker::modelProcessed, this, &LibraryWindow::onModelProcessed);
    connect(indexingWorker, &IndexingWorker::finished, indexingThread, &QThread::quit);
    connect(indexingWorker, &IndexingWorker::finished, indexingWorker, &IndexingWorker::deleteLater);
    connect(indexingThread, &QThread::finished, indexingThread, &QThread::deleteLater);


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
    ui.availableModelsView->setViewMode(QListView::IconMode);
    ui.availableModelsView->setResizeMode(QListView::Adjust);
    ui.availableModelsView->setSpacing(15);
    ui.availableModelsView->setGridSize(QSize(220, 140));
    ui.availableModelsView->setUniformItemSizes(true);
    ui.availableModelsView->setSelectionMode(QAbstractItemView::NoSelection);

    // Configure selected models view
    ui.selectedModelsView->setModel(selectedModelsProxyModel);
    ui.selectedModelsView->setItemDelegate(modelCardDelegate);
    ui.selectedModelsView->setViewMode(QListView::IconMode);
    ui.selectedModelsView->setResizeMode(QListView::Adjust);
    ui.selectedModelsView->setSpacing(15);
    ui.selectedModelsView->setGridSize(QSize(220, 140));
    ui.selectedModelsView->setUniformItemSizes(true);
    ui.selectedModelsView->setSelectionMode(QAbstractItemView::NoSelection);

    // Populate search field combo box
    ui.searchFieldComboBox->addItem("Short Name", QVariant(Model::ShortNameRole));
    ui.searchFieldComboBox->addItem("Title", QVariant(Model::TitleRole));
    ui.searchFieldComboBox->addItem("Author", QVariant(Model::AuthorRole));
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
    connect(modelCardDelegate, &ModelCardDelegate::settingsClicked, this, &LibraryWindow::onSettingsClicked);
    connect(ui.backButton, &QPushButton::clicked, this, &LibraryWindow::onBackButtonClicked);

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
    // Gather selected models
    QList<ModelData> selectedModels;
    for (int row = 0; row < model->rowCount(); ++row) {
        QModelIndex index = model->index(row);
        if (model->data(index, Model::IsSelectedRole).toBool()) {
            selectedModels.append(model->data(index, Qt::UserRole).value<ModelData>());
        }
    }

    // Generate report using selectedModels
    // Implement your report generation logic here
    QMessageBox::information(this, "Report", QString("Generating report for %1 models.").arg(selectedModels.size()));
}

void LibraryWindow::onSettingsClicked(int modelId) {
    // // Open settings dialog for the model
    // ModelData modelData = model->getModelById(modelId);
    // AdvancedOptionsDialog dialog(modelData, this);
    // if (dialog.exec() == QDialog::Accepted) {
    //     model->updateModel(modelId, dialog.getModelData());
    // }
}

void LibraryWindow::onModelProcessed(int modelId) {
    model->refreshModelData();
    availableModelsProxyModel->invalidate();
    selectedModelsProxyModel->invalidate();
}

void LibraryWindow::onBackButtonClicked() {
    // Hide the LibraryWindow
    this->hide();

    // Show the MainWindow
    if (mainWindow) {
        mainWindow->show();
    }

    // Delete the LibraryWindow if it's no longer needed
    this->deleteLater();
}


