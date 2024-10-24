#ifndef PROGRESSWINDOW_H
#define PROGRESSWINDOW_H

#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QWidget>

class ProgressWindow : public QWidget {
  Q_OBJECT

 public:
  ProgressWindow(int totalFiles, QWidget *parent = nullptr);  // Constructor

 public slots:
  void updateProgress();  // Slot to update progress

 private:
  QProgressBar *progressBar;
  QLabel *label;
  int totalFiles;
  int filesProcessed;
};

#endif  // PROGRESSWINDOW_H
