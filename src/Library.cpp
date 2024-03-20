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
