#include "./Library.h"


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
  std::vector<std::string> modelSuffixes = {".g"};
  if (index) {
    return index->findFilesWithSuffixes(modelSuffixes);
  }
  return {};
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
  if (index) {
    return index->findFilesWithSuffixes(geometrySuffixes);
  }
  return {};
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
  if (index) {
    return index->findFilesWithSuffixes(imageSuffixes);
  }
  return {};
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
  if (index) {
    return index->findFilesWithSuffixes(documentSuffixes);
  }
  return {};
}


std::vector<std::string>
Library::getData()
{
  std::vector<std::string> dataSuffixes = {
    ".csv",
    ".hdf5",
    ".json",
    ".mat",
    ".nc",
    ".ods",
    ".vtk",
    ".xls",
    ".xml",
    ".xyz",
    ".zzzdat"
  };
  if (index) {
    return index->findFilesWithSuffixes(dataSuffixes);
  }
  return {};
}
