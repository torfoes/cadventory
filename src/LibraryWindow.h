#ifndef LIBRARYWINDOW_H
#define LIBRARYWINDOW_H

#include <QListWidgetItem>
#include <QObject>
#include <QString>
#include <QStringListModel>
#include <QWidget>
#include <map>
#include <string>
#include <vector>

#include "./Library.h"
#include "./Model.h"
#include "./ui_librarywindow.h"
#include "MainWindow.h"

class LibraryWindow : public QWidget {
  Q_OBJECT

 public:
  explicit LibraryWindow(QWidget *parent = nullptr);
  ~LibraryWindow();

  void loadFromLibrary(Library *library);
  void loadTags();

 protected:
  void updateListModelForDirectory(QStringListModel *model,
                                   const std::vector<std::string> &allItems,
                                   const std::string &directory);

 public slots:
  void on_allLibraries_clicked();
  void onModelSelectionChanged(QListWidgetItem *current,
                               QListWidgetItem *previous);
  void generateReport();

 private slots:
  void on_pushButton_clicked();
  void on_listWidgetPage_itemClicked(QListWidgetItem *item);

 private:
  Ui::LibraryWindow ui;
  Library *library;
  Model *model;

  MainWindow *main;

  QStringListModel *geometryModel;
  QStringListModel *imagesModel;
  QStringListModel *documentsModel;
  QStringListModel *dataModel;

  QStringListModel *tagsModel;
  QListWidget *tagsWidget;
  QStringListModel *currentTagsModel;
  QStringListModel *currentPropertiesModel;

  std::vector<std::string> report;

};

#endif /* LIBRARYWINDOW_H */
