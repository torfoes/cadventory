#ifndef LIBRARY_H
#define LIBRARY_H

#include <QObject>
#include <QThread>
#include <string>
#include <vector>

#include "./Model.h"
#include "FilesystemIndexer.h"

class Library : public QObject {  // Inherit from QObject
  Q_OBJECT                        // Enable signals and slots

      public : explicit Library(const char* label = nullptr,
                                const char* path = nullptr);
  Library(const Library&) = delete;
  ~Library();

  size_t indexFiles();
  const char* name();
  const char* path();

  void createDatabase(QWidget* parent);
  std::vector<std::string> getModelFilePaths();
  std::vector<ModelData> getModels(); // all models
  std::vector<ModelData> getModelsView(); // models filtered and sorted
  std::vector<std::string> getGeometry();
  std::vector<std::string> getImages();
  std::vector<std::string> getDocuments();

  std::vector<std::string> getData();
  std::vector<std::string> getTags();

  void setTagsSelected(std::vector<std::string> tags);
  void setPropertySelected(std::string property);
  void setAscending(bool ascending);

  void setModels();
  void setFullPath(std::string path);
  
  Model* model;
  std::vector<ModelData> models;
  std::string fullPath;

 signals:
  void
  databaseCreationFinished();  // Signal to indicate the processing is complete

 private:
  std::string shortName;

  std::vector<std::string> tagsSelected;
  std::string propertySelected;
  bool ascending;

  std::vector<std::string> tags;
  std::vector<std::string> properties;

  FilesystemIndexer* index;
};

#endif /* FILESYSTEMINDEXER_H */
