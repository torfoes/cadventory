#ifndef LIBRARY_H
#define LIBRARY_H

#include <string>
#include "FilesystemIndexer.h"


class Library {

public:
  explicit Library(const char *label = nullptr, const char *path = nullptr);
  Library(const Library&) = delete;
  ~Library();

  size_t indexFiles();
  const char* name();
  const char* path();

private:
  std::string shortName;
  std::string fullPath;
  FilesystemIndexer* index;
};


#endif /* FILESYSTEMINDEXER_H */
