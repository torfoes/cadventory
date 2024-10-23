// LibraryWindow.cpp

#include "LibraryWindow.h"
#include "IndexingWorker.h"
#include "ProcessGFiles.h"
#include "Model.h"
#include "config.h"

#include <QListWidgetItem>
#include <QFileDialog>
#include <QPdfWriter>
#include <QPainter>
#include <QPixmap>
#include <QStyledItemDelegate>
#include <QThread>
#include <QMetaObject>
#include <QMessageBox>

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

LibraryWindow::LibraryWindow(QWidget* parent) : QWidget(parent)
{
    this->setFixedSize(QSize(876, 600));
    ui.setupUi(this);

    // Connect signals and slots
    connect(ui.listWidget, &QListWidget::currentItemChanged, this, &LibraryWindow::onModelSelectionChanged);
    connect(ui.generateReport, &QPushButton::pressed, this, &LibraryWindow::generateReport);
    connect(ui.allLibraries, &QPushButton::clicked, this, &LibraryWindow::on_allLibraries_clicked);
    connect(ui.listWidgetPage, &QListWidget::itemClicked, this, &LibraryWindow::on_listWidgetPage_itemClicked);
    // connect(ui.generatePreviewButton, &QPushButton::clicked, this, &LibraryWindow::onGeneratePreviewClicked);
}

LibraryWindow::~LibraryWindow()
{
}

void LibraryWindow::generateReport()
{
    // Existing implementation remains unchanged
}

void LibraryWindow::loadFromLibrary(Library* _library)
{
    this->library = _library;
    this->setWindowTitle(library->name() + QString(" Library"));
    this->ui.currentLibrary->setText(library->name());

    // Start fresh
    ui.listWidget->clear();
    ui.listWidgetPage->clear();

    // Start background indexing
    startIndexing();

    // Set up the previews view
    setupPreviewsView();
}

void LibraryWindow::startIndexing()
{
    IndexingWorker* worker = new IndexingWorker(library);
    QThread* thread = new QThread;

    worker->moveToThread(thread);

    connect(thread, &QThread::started, worker, &IndexingWorker::process);
    connect(worker, &IndexingWorker::modelProcessed, this, &LibraryWindow::onModelProcessed);
    connect(worker, &IndexingWorker::finished, this, &LibraryWindow::onIndexingFinished);
    connect(worker, &IndexingWorker::finished, thread, &QThread::quit);
    connect(worker, &IndexingWorker::finished, worker, &IndexingWorker::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    thread->start();
}

void LibraryWindow::onModelProcessed(int modelId)
{
    QMetaObject::invokeMethod(this, [this, modelId]() {
        // Fetch model data
        ModelData modelData = library->model->getModelById(modelId);

        // Update the list widget
        QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(modelData.short_name));
        ui.listWidget->addItem(item);

        // Update previews
        if (!modelData.thumbnail.empty()) {
            QPixmap thumbnail;
            thumbnail.loadFromData(reinterpret_cast<const uchar*>(modelData.thumbnail.data()), static_cast<uint>(modelData.thumbnail.size()), "PNG");
            QListWidgetItem* previewItem = new QListWidgetItem(QIcon(thumbnail), QString::fromStdString(modelData.short_name));
            ui.listWidgetPage->addItem(previewItem);
        }
    }, Qt::QueuedConnection);
}

void LibraryWindow::onIndexingFinished()
{
    QMessageBox::information(this, "Indexing Complete", "All models have been indexed.");
}

void LibraryWindow::on_allLibraries_clicked()
{
    this->hide();
    main->show();
}

void LibraryWindow::onModelSelectionChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
    if (!current)
        return; // Nothing selected

    QString selectedModel = current->text();
    std::string modelPath = library->fullPath + "/" + selectedModel.toStdString();
    int modelId = library->model->hashModel(modelPath);

    ModelData modelData = library->model->getModelById(modelId);

    // Update UI elements to display model details
    ui.modelTitleLabel->setText("Title: " + QString::fromStdString(modelData.title));
    ui.modelAuthorLabel->setText("Author: " + QString::fromStdString(modelData.author));
    ui.modelFilePathLabel->setText("File Path: " + QString::fromStdString(modelData.file_path));

    // Display thumbnail if available
    if (!modelData.thumbnail.empty()) {
        QPixmap thumbnail;
        thumbnail.loadFromData(reinterpret_cast<const uchar*>(modelData.thumbnail.data()), static_cast<uint>(modelData.thumbnail.size()), "PNG");
        ui.modelThumbnailLabel->setPixmap(thumbnail.scaled(100, 100, Qt::KeepAspectRatio));
    } else {
        ui.modelThumbnailLabel->clear();
    }
}

void LibraryWindow::setupPreviewsView()
{
    ui.listWidgetPage->setResizeMode(QListView::Adjust);
    ui.listWidgetPage->setViewMode(QListView::IconMode);
    ui.listWidgetPage->setWordWrap(true);
    ui.listWidgetPage->setIconSize(QSize(72, 72));
    ui.listWidgetPage->setGridSize(QSize(144, 108));
    ui.listWidgetPage->setUniformItemSizes(true);
    ui.listWidgetPage->setMovement(QListView::Static);
    ui.listWidgetPage->setResizeMode(QListView::Adjust);
    ui.listWidgetPage->setLayoutMode(QListView::Batched);
    ui.listWidgetPage->setBatchSize(10);

    QStyledItemDelegate* delegate = new QStyledItemDelegate(this);
    ui.listWidgetPage->setItemDelegate(delegate);
}

void LibraryWindow::on_pushButton_clicked()
{
    report.clear();
    for (int i = 0; i < ui.listWidgetPage->count(); ++i) {
        QListWidgetItem* item = ui.listWidgetPage->item(i);
        if (item->isSelected()) {
            std::string modelPath = library->fullPath + "/" + item->text().toStdString();
            report.push_back(modelPath);
        }
    }
}

void LibraryWindow::on_listWidgetPage_itemClicked(QListWidgetItem* item)
{
    std::string name = item->text().toStdString();
    std::string modelPath = library->fullPath + "/" + name;

    auto it = std::find(report.begin(), report.end(), modelPath);
    if (it != report.end()) {
        report.erase(it);
        item->setSelected(false);
    } else {
        report.push_back(modelPath);
        item->setSelected(true);
    }
}

void LibraryWindow::onGeneratePreviewClicked()
{
    // Existing implementation remains unchanged
}

void LibraryWindow::setMainWindow(MainWindow* mainWindow)
{
    this->main = mainWindow;
}
