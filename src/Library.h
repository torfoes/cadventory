#ifndef LIBRARY_H
#define LIBRARY_H

#include <string>
#include "FilesystemIndexer.h"


class Library {

public:
  explicit Library(const char *label = nullptr, const char *path = nullptr);
  Library(const Library&) = delete;
  ~Library();

  void indexFiles();

private:
  std::string label;
  std::string path;
  FilesystemIndexer *index;
};


#endif /* FILESYSTEMINDEXER_H */
