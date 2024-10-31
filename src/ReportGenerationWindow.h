#ifndef REPORTGENERATIONWINDOW_H
#define REPORTGENERATIONWINDOW_H

#include <QWidget>

#include "Library.h"
#include "Model.h"
#include "ProcessGFiles.h"

namespace Ui {
class ReportGenerationWindow;
}

class ReportGenerationWindow : public QWidget {
  Q_OBJECT

 public:
  explicit ReportGenerationWindow(QWidget* parent = nullptr,
                                  Model* model = nullptr,
                                  Library* library = nullptr);
  ~ReportGenerationWindow();

 private:
  Library* library;
  Model* model;
  Ui::ReportGenerationWindow* ui;
  std::string output_directory;
  std::string logo1_filepath;
  std::string logo2_filepath;
  std::string title;
  std::string username;
  std::string version;
 private slots:
  void onGenerateReportButtonClicked();
  void onOutputDirectoryButtonClicked();
  void onLogo1ButtonClicked();
  void onLogo2ButtonClicked();
};

#endif  // REPORTGENERATIONWINDOW_H