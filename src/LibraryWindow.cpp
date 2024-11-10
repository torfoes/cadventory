// LibraryWindow.cpp

#include "LibraryWindow.h"
#include "IndexingWorker.h"
#include "ProcessGFiles.h"
#include "MainWindow.h"
#include "GeometryBrowserDialog.h"
// #include "AdvancedOptionsDialog.h"
#include "ReportGenerationWindow.h"
#include "ReportGeneratorWorker.h"
#include "FileSystemFilterProxyModel.h"


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

}


LibraryWindow::~LibraryWindow() {
    qDebug() << "LibraryWindow destructor called";

    // Ensure the indexing thread is stopped if it wasn't already
    if (indexingThread && indexingThread->isRunning()) {
        qDebug() << "Waiting for indexingThread to finish in destructor";
        indexingWorker->stop();
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
    ui.currentLibrary->setText(library->name());

    // Load models from the library
    model = library->model;

    includeAllModels();

    // Set the source model for proxy models
    availableModelsProxyModel->setSourceModel(model);
    selectedModelsProxyModel->setSourceModel(model);

    // Set initial filters
    availableModelsProxyModel->setFilterRole(Model::IsSelectedRole);
    availableModelsProxyModel->setFilterFixedString("0"); // Show unselected models

    selectedModelsProxyModel->setFilterRole(Model::IsSelectedRole);
    selectedModelsProxyModel->setFilterFixedString("1"); // Show selected models

    // Now that library is set, set up models and views
    setupModelsAndViews();

    // Set up connections
    setupConnections();

    startIndexing();
}


void LibraryWindow::startIndexing() {
    // Create the indexing worker and thread
    indexingThread = new QThread(this); // Parent is LibraryWindow
    indexingWorker = new IndexingWorker(library);

    // Move the worker to the thread
    indexingWorker->moveToThread(indexingThread);

    // Connect signals and slots
    // connect(indexingThread, &QThread::started, indexingWorker, &IndexingWorker::process);
    // connect(indexingWorker, &IndexingWorker::modelProcessed, this, &LibraryWindow::onModelProcessed);
    // connect(indexingWorker, &IndexingWorker::progressUpdated, this, &LibraryWindow::onProgressUpdated);
    // //connect(indexingThread, &QThread::finished, indexingWorker, &QObject::deleteLater);

    connect(indexingThread, &QThread::started, indexingWorker, &IndexingWorker::process);
    connect(indexingWorker, &IndexingWorker::modelProcessed, this, &LibraryWindow::onModelProcessed);
    connect(indexingWorker, &IndexingWorker::progressUpdated, this, &LibraryWindow::onProgressUpdated);
    connect(indexingWorker, &IndexingWorker::finished, indexingThread, &QThread::quit);



    connect(indexingThread,SIGNAL(finished()),indexingThread,SLOT(deleteLater()));


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

    // Configure selected models view
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

    // Set grid size for selected models view
    ui.selectedModelsView->setGridSize(QSize(1, itemSize.height()));

    // Setup file system model with checkboxes
    QString libraryPath = QString::fromStdString(library->fullPath);
    qDebug() << "Library Path in setupModelsAndViews:" << libraryPath;

    // Create the FileSystemModelWithCheckboxes
    fileSystemModel = new FileSystemModelWithCheckboxes(model, libraryPath, this);
    fileSystemModel->setRootPath(libraryPath);

    // Create and set up the proxy model to filter .g files
    fileSystemProxyModel = new FileSystemFilterProxyModel(libraryPath, this);
    fileSystemProxyModel->setSourceModel(fileSystemModel);
    fileSystemProxyModel->setRecursiveFilteringEnabled(true);

    // Set the model to the tree view
    ui.fileSystemTreeView->setModel(fileSystemProxyModel);

    // Set the root index of the view to the mapped root index
    QModelIndex rootIndex = fileSystemModel->index(libraryPath);
    QModelIndex proxyRootIndex = fileSystemProxyModel->mapFromSource(rootIndex);
    ui.fileSystemTreeView->setRootIndex(proxyRootIndex);
    qDebug() << "Set root index of fileSystemTreeView to proxyRootIndex.";

    // Hide columns other than the name
    for (int i = 1; i < fileSystemModel->columnCount(); ++i) {
        ui.fileSystemTreeView->hideColumn(i);
    }

    // Set uniform row heights for consistent appearance
    ui.fileSystemTreeView->setUniformRowHeights(true);

    // Set icon size (if desired)
    ui.fileSystemTreeView->setIconSize(QSize(24, 24));

    // Connect signals
    connect(fileSystemModel, &QFileSystemModel::directoryLoaded, this, &LibraryWindow::onDirectoryLoaded);
    connect(ui.fileSystemTreeView, &QTreeView::clicked, this, &LibraryWindow::onFileSystemItemClicked);
    connect(fileSystemModel, &FileSystemModelWithCheckboxes::inclusionChanged, this, &LibraryWindow::onInclusionChanged);
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


    connect(modelCardDelegate, &ModelCardDelegate::geometryBrowserClicked,
            this, &LibraryWindow::onGeometryBrowserClicked);

    // Connect filesystem view signals
    connect(ui.fileSystemTreeView, &QTreeView::clicked, this, &LibraryWindow::onFileSystemItemClicked);
    connect(fileSystemModel, &FileSystemModelWithCheckboxes::inclusionChanged, this, &LibraryWindow::onInclusionChanged);

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

                //loadFromLibrary(library);
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



void LibraryWindow::onFileSystemItemClicked(const QModelIndex& index) {
    // Map the proxy index to the source index
    QModelIndex sourceIndex = fileSystemProxyModel->mapToSource(index);

    // QString path = fileSystemModel->filePath(sourceIndex);
    // ui.selectedItemLabel->setText(QString("Selected Item: %1").arg(path));

    // bool included = fileSystemModel->isIncluded(sourceIndex);
    // ui.includeCheckBox->setChecked(included);

    // Connect the checkbox change
    // disconnect(ui.includeCheckBox, &QCheckBox::stateChanged, this, &LibraryWindow::onIncludeCheckBoxStateChanged);
    // connect(ui.includeCheckBox, &QCheckBox::stateChanged, this, &LibraryWindow::onIncludeCheckBoxStateChanged);
}


void LibraryWindow::onIncludeCheckBoxStateChanged(int state) {
    QModelIndex proxyIndex = ui.fileSystemTreeView->currentIndex();
    QModelIndex sourceIndex = fileSystemProxyModel->mapToSource(proxyIndex);

    bool included = (state == Qt::Checked);
    // fileSystemModel->setIncluded(sourceIndex, included);
}


void LibraryWindow::onInclusionChanged(const QModelIndex& index, bool included)
{
    availableModelsProxyModel->invalidate();
    selectedModelsProxyModel->invalidate();
}


void LibraryWindow::onReindexButtonClicked() {
    // Stop any ongoing indexing
    if (indexingThread && indexingThread->isRunning()) {
        indexingWorker->stop();
        indexingThread->wait();
    }

    // Reindex files
    startIndexing();
}

void LibraryWindow::onIndexingComplete() {
    // Refresh model data
    model->refreshModelData();
    availableModelsProxyModel->invalidate();
    selectedModelsProxyModel->invalidate();

    // Update filesystem view checkboxes
    setupModelsAndViews(); // Or specifically update the filesystem model
}


void LibraryWindow::includeAllModels() {
    std::vector<std::string> modelFiles = library->getModels();

    for (const auto& relativePath : modelFiles) {
        std::string fullPath = library->fullPath + "/" + relativePath;
        ModelData modelData = model->getModelByFilePath(fullPath);

        if (modelData.id != 0) {
            if (!modelData.is_included) {
                modelData.is_included = true;
                model->updateModel(modelData.id, modelData);
            }
        } else {
            int id = model->hashModel(fullPath);
            modelData.id = id;
            modelData.file_path = fullPath;
            modelData.is_included = true;
            modelData.is_processed = false;
            model->insertModel(id, modelData);
        }
    }
}

void LibraryWindow::onDirectoryLoaded(const QString& path) {
    Q_UNUSED(path);
    qDebug() << "Directory loaded:" << path;
    fileSystemProxyModel->invalidate();
    ui.fileSystemTreeView->expandAll();
}



