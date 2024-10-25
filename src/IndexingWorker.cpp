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

    for (const auto& filePath : library->getModels()) {
        // Check if a stop has been requested
        if (m_stopRequested.load()) {
            qDebug() << "IndexingWorker::process() stopping due to stop request";
            break;
        }

        std::string fullFilePath = library->fullPath + "/" + filePath;
        int modelId = library->model->hashModel(fullFilePath);

        if (library->model->modelExists(modelId)) {
            ModelData existingModel = library->model->getModelById(modelId);
            if (!existingModel.thumbnail.empty()) {
                continue;
            }
        } else {
            // Model does not exist, create a new ModelData
            ModelData modelData;
            modelData.id = modelId;
            modelData.short_name = fs::path(filePath).stem().string();
            modelData.primary_file = filePath;
            modelData.file_path = fullFilePath;
            modelData.library_name = library->shortName;

            // Insert the model into the database
            library->model->insertModel(modelId, modelData);
        }

        // Use library->model->getHiddenDirectoryPath() to get the hidden directory path
        std::string previewsFolder = library->model->getHiddenDirectoryPath() + "/previews";

        // Process the .g file to extract metadata and generate thumbnails
        processor.processGFile(fullFilePath, previewsFolder);

        // Emit signal indicating that a model has been processed
        emit modelProcessed(modelId);
    }

    // Emit finished signal to indicate processing is complete
    emit finished();
    qDebug() << "IndexingWorker::process() finished";
}
