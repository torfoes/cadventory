#ifndef REPORTGENERATIONWINDOW_H
#define REPORTGENERATIONWINDOW_H

#include <QWidget>
#include <QPainter>
#include <QPdfWriter>
#include <QThread>
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

 private slots:
  void onGenerateReportButtonClicked();
  void onOutputDirectoryButtonClicked();
  void onLogo1ButtonClicked();
  void onLogo2ButtonClicked();
  void onProcessingGistCall(const QString& file);
  void onSuccessfulGistCall(const QString& path_gist_output);
  void onFailedGistCall(const QString& filepath, const QString& errorMessage);
  void onFinishedGeneratingReport();

 private:
  int* num_file;
  int* tot_num_files;
  int x = 325;
  int y = 400;
  QPdfWriter* pdfWriter;
  QPainter* painter;
  Library* library;
  Model* model;
  Ui::ReportGenerationWindow* ui;
  std::string output_directory;
  std::string logo1_filepath;
  std::string logo2_filepath;
  std::string title;
  std::string username;
  std::string version;
  std::vector<std::string> *err_vec;
};

#endif  // REPORTGENERATIONWINDOW_H
