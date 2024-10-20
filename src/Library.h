#ifndef LIBRARY_H
#define LIBRARY_H

#include <QObject>
#include <QThread>
#include <string>
#include <vector>
#include "FilesystemIndexer.h"
#include "./Model.h"


class Library: public QObject {  // Inherit from QObject
    Q_OBJECT  // Enable signals and slots

public:
  explicit Library(const char *label = nullptr, const char *path = nullptr);
  Library(const Library&) = delete;
  ~Library();

  size_t indexFiles();
  const char* name();
  const char* path();

  void createDatabase(QWidget *parent);
  std::vector<std::string> getModels();
  std::vector<std::string> getGeometry();
  std::vector<std::string> getImages();
  std::vector<std::string> getDocuments();
  std::vector<std::string> getData();
  std::vector<std::string> getTags();
  
  Model* model;
  std::string fullPath;

signals:
  void databaseCreationFinished();  // Signal to indicate the processing is complete

private:
  std::string shortName;
  
  FilesystemIndexer* index;
  
};


#endif /* FILESYSTEMINDEXER_H */
