#ifndef LIBRARYWINDOW_H
#define LIBRARYWINDOW_H

#include <map>
#include <string>
#include <vector>

#include <QWidget>
#include <QObject>
#include <QListWidgetItem>
#include <QStringListModel>
#include <QFileSystemModel>
#include <QAbstractItemView>
#include <QColumnView>
#include <QLayout>
#include <QListView>
#include <QProcess>
#include <QTableView>
#include <QTreeView>


#include "./ui_librarywindow.h"

#include "./Library.h"
#include "MainWindow.h"


class LibraryWindow : public QWidget
{
  Q_OBJECT

public:
  MainWindow *Main;
  explicit LibraryWindow(QWidget* parent = nullptr);
  ~LibraryWindow();

  int add_page_to_tabpanel(QString dir, const QString &label);
  enum class CDSource { Navbar, Navpage, Navtree, Navbutton, Tabchange };

  void loadFromLibrary(Library *library);

protected:
  void updateListModelForDirectory(QStringListModel* model, const std::vector<std::string>& allItems, const std::string& directory);

public slots:
  void on_allLibraries_clicked();
  void onModelSelectionChanged(QListWidgetItem *current, QListWidgetItem *previous);
  void on_actionNew_tab_triggered();

private:
  Ui::LibraryWindow ui;
  Library* library;

  QStringListModel *geometryModel;
  QStringListModel *imagesModel;
  QStringListModel *documentsModel;
  QStringListModel *dataModel;
  QFileSystemModel *FSmodel = new QFileSystemModel();
  QList<QString> visitedPaths;
  bool check_n_change_dir(const QString &path, CDSource source, bool suppress_warning = false);
  void show_warning(const QString &message);


  friend class Navpage;
};


#endif /* LIBRARYWINDOW_H */
