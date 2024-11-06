// LibraryWindow.cpp

#include "LibraryWindow.h"
#include "IndexingWorker.h"
#include "ProcessGFiles.h"
#include "MainWindow.h"
#include "GeometryBrowserDialog.h"
// #include "AdvancedOptionsDialog.h"
#include "ReportGenerationWindow.h"
#include "ReportGeneratorWorker.h"
#include <QThread>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListView>
#include <QPushButton>
#include <QComboBox>
#include <QMenuBar>
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
        indexingThread->requestInterruption();
        indexingThread->quit();
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
    indexingWorker = new IndexingWorker(library,true);//mainWindow->previewFlag);

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
    reload = new QAction(tr("&Reload"),this);

    this->mainWindow->editMenu->addAction(reload);
    connect(reload,&QAction::triggered,this,&LibraryWindow::reloadLibrary);

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
    if (model->getSelectedModels().empty()) {
        QMessageBox::information(this, "Report",
                                 "No models selected for the report.");
        return;
    }

    ReportGenerationWindow* window =
        new ReportGenerationWindow(nullptr, model, library);
    window->show();
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
        this->mainWindow->editMenu->removeAction(reload);
        disconnect(reload,0,0,0);
        this->mainWindow->returnCentralWidget();
        //mainWindow->show();
        qDebug() << "MainWindow shown";
    } else {
        qDebug() << "mainWindow is null";
    }

}


void LibraryWindow::reloadLibrary() {

    namespace fs = std::filesystem;
    std::string path = library->fullPath + "/.cadventory/metadata.db";
    fs::path filePath(path);

    qDebug() << filePath.string();

    // Check if the file exists
    if (fs::exists(filePath)) {
        // Try to remove the file
        try {
            if (fs::remove(filePath)) {
                std::cout << "File 'metadata.db' successfully deleted." << std::endl;




                startIndexing();
            } else {
                std::cout << "Failed to delete 'metadata.db'." << std::endl;
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    } else {
        std::cout << "File 'metadata.db' does not exist." << std::endl;
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
