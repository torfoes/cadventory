#include "./Library.h"


Library::Library(const char* _label, const char* _path) :
  label(_label),
  path(_path),
  index(nullptr)
{
}


Library::~Library()
{
  delete index;
}


void
Library::indexFiles()
{
  index = new FilesystemIndexer(path.c_str());
}
