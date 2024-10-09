#ifndef LIBRARY_H
#define LIBRARY_H

#include <string>
#include <vector>
#include "FilesystemIndexer.h"
#include "./Model.h"


class Library {

public:
  explicit Library(const char *label = nullptr, const char *path = nullptr);
  Library(const Library&) = delete;
  ~Library();

  size_t indexFiles();
  const char* name();
  const char* path();

  std::vector<std::string> getModels();
  std::vector<std::string> getGeometry();
  std::vector<std::string> getImages();
  std::vector<std::string> getDocuments();
  std::vector<std::string> getData();
  std::vector<std::string> getTags();
  
  Model* model;
  std::string fullPath;

private:
  std::string shortName;
  
  FilesystemIndexer* index;
  
};


#endif /* FILESYSTEMINDEXER_H */
