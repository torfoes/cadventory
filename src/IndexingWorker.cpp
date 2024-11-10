#include "IndexingWorker.h"
#include "ProcessGFiles.h"
#include "Model.h"
#include <QDebug>
#include <filesystem>

namespace fs = std::filesystem;

IndexingWorker::IndexingWorker(Library* library, QObject* parent)
    : QObject(parent), library(library), m_stopRequested(false) {}

void IndexingWorker::stop() {
    qDebug() << "IndexingWorker::stop() called";
    m_stopRequested.store(true);
}

void IndexingWorker::process() {
    qDebug() << "IndexingWorker::process() started";
    ProcessGFiles processor(library->model);

    // Get the previews folder path
    std::string previewsFolder = library->model->getHiddenDirectoryPath() + "/previews";

    auto models = library->getModels();
    int totalFiles = models.size();
    int processedFiles = 0;

    if (totalFiles == 0) {
        emit progressUpdated("No files to process", 100);
        emit finished();
        return;
    }

    for (const auto& filePath : models) {
        // Check if a stop has been requested
        if (m_stopRequested.load()) {
            qDebug() << "IndexingWorker::process() stopping due to stop request";
            break;
        }

        std::string fullFilePath = library->fullPath + "/" + filePath;

        // Get the modelId for emitting the signal later
        int modelId = library->model->hashModel(fullFilePath);

        // Process the .g file to extract metadata and generate thumbnails
        processor.processGFile(fullFilePath, previewsFolder, library->shortName);

        // Increment processed files count
        processedFiles++;

        // Calculate percentage
        int percentage = (processedFiles * 100) / totalFiles;

        // Emit progress signal after processing the file
        QString currentObject = QString::fromStdString(filePath);
        emit progressUpdated(currentObject, percentage);

        // Emit signal indicating that a model has been processed
        emit modelProcessed(modelId);
    }

    // Emit final progress signal to indicate completion
    emit progressUpdated("Processing complete", 100);

    // Emit finished signal to indicate processing is complete
    emit finished();
    qDebug() << "IndexingWorker::process() finished";
}
