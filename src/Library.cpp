#include "Library.h"

Library::Library(const char *_path) :
  path(_path)
{
  index = new FilesystemIndexer(_path);
}

Library::~Library()
{
  delete index;
}

