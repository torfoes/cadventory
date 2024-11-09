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

    // Retrieve models that are included
    std::vector<ModelData> modelsToProcess = library->model->getIncludedModels();
    int totalFiles = modelsToProcess.size();
    int processedFiles = 0;

    if (totalFiles == 0) {
        emit progressUpdated("No files to process", 100);
        emit finished();
        return;
    }

    for (const auto& modelData : modelsToProcess) {
        // Check if a stop has been requested
        if (m_stopRequested.load()) {
            qDebug() << "IndexingWorker::process() stopping due to stop request";
            break;
        }

        std::string fullFilePath = modelData.file_path;

        // Process the .g file to extract metadata and generate thumbnails
        processor.processGFile(fullFilePath, previewsFolder);

        // Increment processed files count
        processedFiles++;

        // Calculate percentage
        int percentage = (processedFiles * 100) / totalFiles;

        // Emit progress signal after processing the file
        QString currentObject = QString::fromStdString(modelData.file_path);
        emit progressUpdated(currentObject, percentage);

        // Emit signal indicating that a model has been processed
        emit modelProcessed(modelData.id);
    }

    // Emit final progress signal to indicate completion
    emit progressUpdated("Processing complete", 100);

    // Emit finished signal to indicate processing is complete
    emit finished();
    qDebug() << "IndexingWorker::process() finished";
}
