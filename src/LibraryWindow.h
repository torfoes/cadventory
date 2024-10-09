#ifndef LIBRARYWINDOW_H
#define LIBRARYWINDOW_H

#include <map>
#include <string>
#include <vector>

#include <QWidget>
#include <QObject>
#include <QListWidgetItem>
#include <QStringListModel>

#include "./ui_librarywindow.h"

#include "./Library.h"
#include "./Model.h"


class LibraryWindow : public QWidget
{
  Q_OBJECT

public:
  explicit LibraryWindow(QWidget* parent = nullptr);
  ~LibraryWindow();

  void loadFromLibrary(Library *library);
  void loadTags();

protected:
  void updateListModelForDirectory(QStringListModel* model, const std::vector<std::string>& allItems, const std::string& directory);

public slots:
  void on_allLibraries_clicked();
  void onModelSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous);

private:
  Ui::LibraryWindow ui;
  Library* library;
  Model* model;

  QStringListModel *geometryModel;
  QStringListModel *imagesModel;
  QStringListModel *documentsModel;
  QStringListModel *dataModel;

  QStringListModel *tagsModel;
  QStringListModel *currentTagsModel;
};


#endif /* LIBRARYWINDOW_H */
