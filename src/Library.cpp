#include "./Library.h"

#include <QtConcurrent/QtConcurrent>
#include <algorithm>
#include <iostream>
#include <set>

#include "./Model.h"
#include "Model.h"
#include "./ProcessGFiles.h"
#include "ProgressWindow.h"

Library::Library(const char* _label, const char* _path)
    : shortName(_label),
      fullPath(_path),
      index(nullptr),
      model(new Model("metadata.db")) {}

void Library::createDatabase(QWidget* parent) {
  ProcessGFiles gFileProcessor;
  std::vector<std::string> allModels = getModelFilePaths();
  std::map<std::string, std::string> results;
  int totalFiles = allModels.size();

  ProgressWindow* progressWindow = new ProgressWindow(totalFiles, parent);
  progressWindow->show();

  int processedFiles = 0;

  for (const std::string& filePath : allModels) {
    int modelId = model->hashModel(fullPath + "/" + filePath);
    if (!model->hasProperties(modelId)) {
      ModelData data{.id = modelId,
                     .short_name = filePath,
                     .path = fullPath + "/" + filePath,
                     .primary_file_path = "",
                     .library = shortName,
                     .override_info = ""};

      // model->insertModel(fullPath + "/" + filePath, "short_name", filePath,
      //                    "primary_file"
      //                    "overrides",
      //                    shortName);
      model->insertModel(data);

      model->insertProperties(
          modelId, gFileProcessor.processGFile(fullPath + "/" + filePath));

      std::string fileName = filePath.substr(filePath.find_last_of("/\\") + 1);
      size_t mid = fileName.size() / 2;

      model->addTagToModel(modelId, fileName.substr(0, mid));
      model->addTagToModel(modelId, fileName.substr(mid));

      std::string author = model->getProperty(modelId, "author");

      model->insertProperty(modelId, "file_path", fullPath + "/" + filePath);
      model->insertProperty(modelId, "library_name", shortName);
      model->insertProperty(modelId, "author", fullPath + "/author/" + "kotda");

      processedFiles++;
      progressWindow->updateProgress();
    }
  }
  // Close the progress window when done
  progressWindow->close();

  // Emit a signal when the database creation is finished
  std::cout << "Emitting signal for database creation finished" << std::endl;
  emit databaseCreationFinished();
}

Library::~Library() { delete index; }

const char* Library::name() { return shortName.c_str(); }

const char* Library::path() { return fullPath.c_str(); }

size_t Library::indexFiles() {
  index = new FilesystemIndexer(fullPath.c_str());
  return index->indexed();
}

std::vector<std::string> Library::getModelFilePaths() {
  std::vector<std::string> modelSuffixes = {".g"};
  std::set<std::string> uniqueFiles;

  if (!index) {
    indexFiles();
  }

  auto files = index->findFilesWithSuffixes(modelSuffixes);
  for (const std::string& file : files) {
    std::string relativePath = file;
    if (file.size() >= fullPath.size() &&
        file.compare(0, fullPath.size(), fullPath) == 0) {
      relativePath = file.substr(fullPath.size());
      if (relativePath.size() > 0 &&
          (relativePath[0] == '/' || relativePath[0] == '\\')) {
        relativePath = relativePath.substr(1);
      }
    }
    uniqueFiles.insert(relativePath);
  }
  std::vector<std::string> ret =
      std::vector<std::string>(uniqueFiles.begin(), uniqueFiles.end());
  std::cout << "Number of unique model files: " << uniqueFiles.size()
            << std::endl;
  return ret;
}

// gets the models from the database in this
std::vector<ModelData> Library::getModels() {
  std::vector<ModelData> allModels = model->getModels();
  std::cout << "Total number of models: " << allModels.size() << std::endl;
  std::vector<ModelData> libraryModels;

  for (const ModelData& modelData : allModels) {
    int modelId = modelData.id;
    std::string libraryName = model->getProperty(modelId, "library_name");
    if (libraryName == shortName) {
      libraryModels.push_back(modelData);
    }
  }

  return libraryModels;
}

std::vector<ModelData> Library::getModelsView() {
  std::vector<ModelData> filteredModels;

  for (const ModelData& modelData : models) {
    int modelId = modelData.id;
    std::vector<std::string> modelTags = model->getTagsForModel(modelId);

    bool hasAllTags =
        std::all_of(tagsSelected.begin(), tagsSelected.end(),
                    [&modelTags](const std::string& tag) {
                      return std::find(modelTags.begin(), modelTags.end(),
                                       tag) != modelTags.end();
                    });

    if (hasAllTags) {
      filteredModels.push_back(modelData);
    }
  }

  struct ModelInfo {
    ModelData modelData;
    bool hasProperty;
    std::string propertyValue;
  };

  std::vector<ModelInfo> modelsWithProperties;
  for (const ModelData& modelData : filteredModels) {
    int modelId = modelData.id;
    std::string propertyValue = model->getProperty(modelId, propertySelected);
    bool hasProperty = !propertyValue.empty();
    modelsWithProperties.push_back({modelData, hasProperty, propertyValue});
  }

  std::sort(modelsWithProperties.begin(), modelsWithProperties.end(),
            [this](const ModelInfo& a, const ModelInfo& b) {
              if (a.hasProperty && b.hasProperty) {
                if (ascending)
                  return a.propertyValue < b.propertyValue;
                else
                  return a.propertyValue > b.propertyValue;
              } else if (a.hasProperty != b.hasProperty) {
                return a.hasProperty;
              } else {
                return a.modelData.short_name < b.modelData.short_name;
              }
            });

  std::vector<ModelData> sortedModels;
  for (const auto& modelInfo : modelsWithProperties) {
    sortedModels.push_back(modelInfo.modelData);
  }

  return sortedModels;
}

std::vector<std::string> Library::getGeometry() {
  std::vector<std::string> geometrySuffixes = {".3dm",
                                               ".3ds",
                                               ".3mf",
                                               ".amf"
                                               ".asc",
                                               ".asm",
                                               ".brep",
                                               ".c4d",
                                               ".cad",
                                               ".catpart",
                                               ".catproduct",
                                               ".cfdesign",
                                               ".dae",
                                               ".drw",
                                               ".dwg",
                                               ".dxf",
                                               ".easm",
                                               ".fbx",
                                               ".fcstd",
                                               ".g",
                                               ".glb",
                                               ".gltf",
                                               ".iam",
                                               ".ifc",
                                               ".iges",
                                               ".igs",
                                               ".ipt",
                                               ".jt",
                                               ".mgx",
                                               ".nx",
                                               ".obj",
                                               ".par",
                                               ".ply",
                                               ".ply",
                                               ".prt",
                                               ".rvt",
                                               ".sab",
                                               ".sat",
                                               ".scad",
                                               ".scdoc",
                                               ".skp",
                                               ".sldasm",
                                               ".slddrw",
                                               ".sldprt",
                                               ".step",
                                               ".stl",
                                               ".stp",
                                               ".u3d",
                                               ".vda",
                                               ".wrp",
                                               ".x_b",
                                               ".x_t",
                                               ".zpr",
                                               ".zzzgeo"};
  if (!index) {
    indexFiles();
  }

  return index->findFilesWithSuffixes(geometrySuffixes);
}

std::vector<std::string> Library::getImages() {
  std::vector<std::string> imageSuffixes = {
      ".bmp", ".bw",   ".cgm",  ".dds",   ".dpx", ".exr", ".gif",
      ".hdr", ".jpeg", ".jpg",  ".pbm",   ".pix", ".png", ".ppm",
      ".psd", ".ptx",  ".raw",  ".rgb",   ".sgi", ".svg", ".tga",
      ".tif", ".tiff", ".webp", ".zzzimg"};

  if (!index) {
    indexFiles();
  }

  return index->findFilesWithSuffixes(imageSuffixes);
}

std::vector<std::string> Library::getDocuments() {
  std::vector<std::string> documentSuffixes = {
      ".doc", ".docx", ".md",  ".odp",  ".odt", ".pdf",
      ".ppt", ".pptx", ".rtf", ".rtfd", ".txt", ".zzzdoc"};

  if (!index) {
    indexFiles();
  }

  return index->findFilesWithSuffixes(documentSuffixes);
}

std::vector<std::string> Library::getData() {
  std::vector<std::string> dataSuffixes = {
      ".Z",   ".bz2", ".csv", ".hdf5", ".json", ".mat", ".nc",  ".ods",
      ".tar", ".tgz", ".vtk", ".xls",  ".xml",  ".xyz", ".zip", ".zzzdat"};

  if (!index) {
    indexFiles();
  }

  return index->findFilesWithSuffixes(dataSuffixes);
}

std::vector<std::string> Library::getTags() {
  std::cout << "Getting tags for library: " << shortName << std::endl;
  std::unordered_map<std::string, int> tagCount;
  std::vector<std::string> allTags;

  for (const auto& modelData : models) {
    std::vector<std::string> tags = model->getTagsForModel(modelData.id);

    for (const auto& tag : tags) {
      tagCount[tag]++;
    }
  }

  for (const auto& tagPair : tagCount) {
    allTags.push_back(tagPair.first);
  }

  std::sort(allTags.begin(), allTags.end(),
            [&tagCount](const std::string& a, const std::string& b) {
              return tagCount[a] > tagCount[b];
            });

  return allTags;
}

void Library::setModels() { models = getModels(); }