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
    m_stopRequested.store(true); // Set the stop flag to true in a thread-safe manner
}

void IndexingWorker::process() {
    qDebug() << "IndexingWorker::process() started";
    ProcessGFiles processor(library->model);

    // get the previews folder path
    std::string previewsFolder = library->model->getHiddenDirectoryPath() + "/previews";

    for (const auto& filePath : library->getModels()) {
        // Check if a stop has been requested
        if (m_stopRequested.load()) {
            qDebug() << "IndexingWorker::process() stopping due to stop request";
            break;
        }

        std::string fullFilePath = library->fullPath + "/" + filePath;

        // get the modelId for emitting the signal later
        int modelId = library->model->hashModel(fullFilePath);

        // Process the .g file to extract metadata and generate thumbnails
        processor.processGFile(fullFilePath, previewsFolder, library->shortName);

        // emit signal indicating that a model has been processed
        emit modelProcessed(modelId);
    }

    // emit finished signal to indicate processing is complete
    emit finished();
    qDebug() << "IndexingWorker::process() finished";
}
