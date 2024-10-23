// IndexingWorker.cpp

#include "IndexingWorker.h"
#include "ProcessGFiles.h"
#include "Model.h"

#include <filesystem>

namespace fs = std::filesystem;

IndexingWorker::IndexingWorker(Library* library) : library(library) {}

void IndexingWorker::process() {
    ProcessGFiles processor(library->model);

    for (const auto& filePath : library->getModels()) {
        std::string fullFilePath = library->fullPath + "/" + filePath;
        int modelId = library->model->hashModel(fullFilePath);

        if (!library->model->modelExists(modelId)) {
            // Create a ModelData object
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

        emit modelProcessed(modelId);
    }

    emit finished();
}
