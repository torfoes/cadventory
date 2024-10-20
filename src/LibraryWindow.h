#ifndef LIBRARYWINDOW_H
#define LIBRARYWINDOW_H

#include <map>
#include <string>
#include <vector>

#include <QWidget>
#include <QObject>
#include <QListWidgetItem>
#include <QStringListModel>
#include <QString>

#include "./ui_librarywindow.h"
#include "MainWindow.h"


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

  // private slots:
  // void on_listWidgetPage_itemClicked(QListWidgetItem *item);

  private:
  Ui::LibraryWindow ui;
  Library* library;
  Model* model;

  MainWindow* main;

  QStringListModel *geometryModel;
  QStringListModel *imagesModel;
  QStringListModel *documentsModel;
  QStringListModel *dataModel;

  QStringListModel *tagsModel;
  QListWidget *tagsWidget;
  QStringListModel *currentTagsModel;
  QStringListModel *currentPropertiesModel;

  //void AddItem(const QString& qstrFileName, const QString& qstrPic);

};


#endif /* LIBRARYWINDOW_H */
