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


class LibraryWindow : public QWidget
{
  Q_OBJECT

public:
  explicit LibraryWindow(QWidget* parent = nullptr);
  ~LibraryWindow();

  void loadFromLibrary(Library *library);

protected:
  void updateListModelForDirectory(QStringListModel* model, const std::vector<std::string>& allItems, const std::string& directory);

public slots:
  void on_allLibraries_clicked();
  void onModelSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous);

private:
  Ui::LibraryWindow ui;
  Library* library;

  QStringListModel *geometryModel;
  QStringListModel *imagesModel;
  QStringListModel *documentsModel;
  QStringListModel *dataModel;
};


#endif /* LIBRARYWINDOW_H */
