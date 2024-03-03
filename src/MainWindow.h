#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>

#include "./ui_mainwindow.h"


class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

public slots:
  void updateStatusLabel(const char *status);
  void on_addLibraryButton_clicked();

private:
  Ui::MainWindow ui;
};

#endif /* MAINWINDOW_H */
