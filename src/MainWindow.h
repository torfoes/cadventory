#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <map>
#include <string>
#include <vector>

#include "./Library.h"
#include "./ui_mainwindow.h"

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

  void addLibrary(const char* label = nullptr, const char* path = nullptr);
  void openLibrary();

 public slots:
  void updateStatusLabel(const char* status);
  void on_addLibraryButton_clicked();

 private slots:
  void on_homeLibraryButton_clicked();

 protected:
  size_t saveState();
  size_t loadState();

  void addLibraryButton(const char* label = nullptr,
                        const char* path = nullptr);

 private:
  Ui::MainWindow ui;
  std::vector<Library*> libraries;
};

#endif /* MAINWINDOW_H */
