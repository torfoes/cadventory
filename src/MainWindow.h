#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <map>
#include <string>
#include <vector>
#include <filesystem>

#include <QFileSystemWatcher>
#include <QMainWindow>
#include <QObject>
#include <QtSql>
#include "./ui_mainwindow.h"

#include "./Library.h"


class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

  void addLibrary(const char* label = nullptr, const char* path = nullptr);
  void openLibrary();


public slots:
  void showModified(const QString& str);
  void updateStatusLabel(const char* status);
  void on_addLibraryButton_clicked();
  void openAuditLog();

protected:
  size_t saveState();
  size_t loadState();

  void addLibraryButton(const char* label = nullptr, const char* path = nullptr);

private:
  QSqlDatabase db;
  // QFileSystemWatcher* watcher;
  Ui::MainWindow ui;
  std::vector<Library*> libraries;
};

#endif /* MAINWINDOW_H */
