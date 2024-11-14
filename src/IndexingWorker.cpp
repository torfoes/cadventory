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

    // Retrieve models that are included
    std::vector<ModelData> modelsToProcess = library->model->getIncludedNotProcessedModels();
    int totalFiles = modelsToProcess.size();
    int processedFiles = 0;

    if (totalFiles == 0) {
        emit progressUpdated("No files to process", 100);
        emit finished();
        return;
    }

    for (const auto& modelData : modelsToProcess) {
        if (m_stopRequested.load()) {
            qDebug() << "IndexingWorker::process() stopping due to stop request";
            break;
        }
        int percentage = (processedFiles * 100) / totalFiles;

        // emit progress signal before processing the file
        QString currentObject = QString::fromStdString(modelData.short_name);
        emit progressUpdated(currentObject, percentage);
        emit modelProcessed(modelData.id);

        processor.processGFile(modelData);
        processedFiles++;
    }

    // emit final progress signal to indicate completion
    emit progressUpdated("Processing complete", 100);
    emit finished();
    qDebug() << "IndexingWorker::process() finished";
}
