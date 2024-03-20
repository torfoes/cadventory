#include "./Library.h"

#include <set>
#include <algorithm>


Library::Library(const char* _label, const char* _path) :
  shortName(_label),
  fullPath(_path),
  index(nullptr)
{
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
  /* care about dirs with a .g in them */
  std::vector<std::string> modelSuffixes = {".g"};
  std::set<std::string> uniqueDirs; // using set to avoid dupes

  if (!index) {
    indexFiles();
  }

  auto files = index->findFilesWithSuffixes(modelSuffixes);
  for (const std::string& file : files) {
    // remove the filename
    size_t lastSlashPos = file.find_last_of("/\\");
    std::string dirPath = file.substr(0, lastSlashPos);

    // make path relative to fullPath
    if (dirPath.size() >= fullPath.size() && dirPath.compare(0, fullPath.size(), fullPath) == 0) {
      dirPath = dirPath.substr(fullPath.size());
      if (dirPath.size() > 0 && (dirPath[0] == '/' || dirPath[0] == '\\')) {
        // remove leading slash
        dirPath = dirPath.substr(1);
      }
    }

    // convert to "." if the directory is the same as the library path
    if (dirPath.empty()) {
      dirPath = ".";
    }

    uniqueDirs.insert(dirPath);
  }

  // convert set back to vector
  return std::vector<std::string>(uniqueDirs.begin(), uniqueDirs.end());
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
