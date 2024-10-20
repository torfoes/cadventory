#include "./Library.h"
#include "./Model.h"
#include "./ProcessGFiles.h"

#include <set>
#include <algorithm>
#include <iostream>

Library::Library(const char* _label, const char* _path) :
  shortName(_label),
  fullPath(_path),
  index(nullptr),
  model(new Model("metadata.db"))
{
  
}

void Library::loadDatabase() {
  // for (const std::string& filePath : getModels()) {
  //   model->insertModel(fullPath+"/"+filePath, filePath, "primary_file", "overrides");
    
  //   // Split the file name into two halves and use them as tags
  //   std::string fileName = filePath.substr(filePath.find_last_of("/\\") + 1);
  //   size_t mid = fileName.size() / 2;
  //   int modelId = model->hashModel(fullPath + "/" + filePath);
    
  //   model->addTagToModel(modelId, fileName.substr(0, mid));
  //   model->addTagToModel(modelId, fileName.substr(mid));

  //   std::string author = model->getProperty(modelId, "author");

  //   model->insertProperty(modelId, "file_path", fullPath + "/" + filePath);
  //   model->insertProperty(modelId, "library_name", shortName);
  //   model->insertProperty(modelId, "author", fullPath+"/author/"+"kotda");
  // }

  ProcessGFiles gFileProcessor;
  std::map<std::string, std::string> results;
  // gFileProcessor.executeMultiThreadedProcessing(getModels(), 4);

  for (const std::string& filePath : getModels()) {
    int modelId = model->hashModel(fullPath + "/" + filePath);
    if(!model->hasProperties(modelId)) {
      model->insertModel(fullPath+"/"+filePath, filePath, "primary_file", "overrides");
      model->insertProperties(modelId, gFileProcessor.processGFile(fullPath + "/" + filePath));

      // Split the file name into two halves and use them as tags
      std::string fileName = filePath.substr(filePath.find_last_of("/\\") + 1);
      size_t mid = fileName.size() / 2;
      int modelId = model->hashModel(fullPath + "/" + filePath);
      
      model->addTagToModel(modelId, fileName.substr(0, mid));
      model->addTagToModel(modelId, fileName.substr(mid));

      std::string author = model->getProperty(modelId, "author");

      model->insertProperty(modelId, "file_path", fullPath + "/" + filePath);
      model->insertProperty(modelId, "library_name", shortName);
      model->insertProperty(modelId, "author", fullPath+"/author/"+"kotda");
    }
    
  }


}


Library::~Library()
{
  delete index;
}


const char*
Library::name()
{
  return shortName.c_str();
}


const char*
Library::path()
{
  return fullPath.c_str();
}


size_t
Library::indexFiles()
{
  index = new FilesystemIndexer(fullPath.c_str());
  return index->indexed();
}



std::vector<std::string>
Library::getModels()
{
  /* care about files with a .g extension */
  std::vector<std::string> modelSuffixes = {".g"};
  std::set<std::string> uniqueFiles; // using set to avoid duplicates

  if (!index) {
    indexFiles();
  }

  auto files = index->findFilesWithSuffixes(modelSuffixes);
  for (const std::string& file : files) {
    // make path relative to fullPath
    std::string relativePath = file;
    if (file.size() >= fullPath.size() && file.compare(0, fullPath.size(), fullPath) == 0) {
      relativePath = file.substr(fullPath.size());
      if (relativePath.size() > 0 && (relativePath[0] == '/' || relativePath[0] == '\\')) {
        // remove leading slash
        relativePath = relativePath.substr(1);
      }
    }

    uniqueFiles.insert(relativePath);
  }

  std::vector<std::string> filePaths(uniqueFiles.begin(), uniqueFiles.end());

  // for (const std::string& filePath : filePaths) {
  //   model->insertModel(fullPath+"/"+filePath, filePath, "primary_file", "overrides");
  // }

  return filePaths;
}


std::vector<std::string>
Library::getGeometry()
{
  std::vector<std::string> geometrySuffixes = {
    ".3dm",
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
    ".zzzgeo"
  };
  if (!index) {
    indexFiles();
  }

  return index->findFilesWithSuffixes(geometrySuffixes);
}


std::vector<std::string>
Library::getImages()
{
  std::vector<std::string> imageSuffixes = {
    ".bmp",
    ".bw",
    ".cgm",
    ".dds",
    ".dpx",
    ".exr",
    ".gif",
    ".hdr",
    ".jpeg",
    ".jpg",
    ".pbm",
    ".pix",
    ".png",
    ".ppm",
    ".psd",
    ".ptx",
    ".raw",
    ".rgb",
    ".sgi",
    ".svg",
    ".tga",
    ".tif",
    ".tiff",
    ".webp",
    ".zzzimg"
  };

  if (!index) {
    indexFiles();
  }

  return index->findFilesWithSuffixes(imageSuffixes);
}


std::vector<std::string>
Library::getDocuments()
{
  std::vector<std::string> documentSuffixes = {
    ".doc",
    ".docx",
    ".md",
    ".odp",
    ".odt",
    ".pdf",
    ".ppt",
    ".pptx",
    ".rtf",
    ".rtfd",
    ".txt",
    ".zzzdoc"
  };

  if (!index) {
    indexFiles();
  }

  return index->findFilesWithSuffixes(documentSuffixes);
}


std::vector<std::string>
Library::getData()
{
  std::vector<std::string> dataSuffixes = {
    ".Z",
    ".bz2",
    ".csv",
    ".hdf5",
    ".json",
    ".mat",
    ".nc",
    ".ods",
    ".tar",
    ".tgz",
    ".vtk",
    ".xls",
    ".xml",
    ".xyz",
    ".zip",
    ".zzzdat"
  };

  if (!index) {
    indexFiles();
  }

  return index->findFilesWithSuffixes(dataSuffixes);
}

std::vector<std::string>
Library::getTags()
{
  std::cout << "Getting tags for library: " << shortName << std::endl;
  std::unordered_map<std::string, int> tagCount;
  std::vector<std::string> allTags;

  for (const auto& filePath : getModels()) {
    size_t modelHash = model->hashModel(fullPath+"/"+filePath);
    std::vector<std::string> tags = model->getTagsForModel(modelHash);

    for (const auto& tag : tags) {
      tagCount[tag]++;
    }
  }

  for (const auto& tagPair : tagCount) {
    allTags.push_back(tagPair.first);
  }

  std::sort(allTags.begin(), allTags.end(), [&tagCount](const std::string& a, const std::string& b) {
    return tagCount[a] > tagCount[b];
  });

  return allTags;
}