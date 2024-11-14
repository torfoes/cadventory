#ifndef REPORTGENERATIONWINDOW_H
#define REPORTGENERATIONWINDOW_H

#include <QPainter>
#include <QPdfWriter>
#include <QThread>
#include <QWidget>

#include "ReportGeneratorWorker.h"
#include "Library.h"
#include "Model.h"
#include "ProcessGFiles.h"
#define A4_MAXWIDTH_LS 3508
#define A4_MAXHEIGHT_LS 2480


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
  void onFailedGistCall(const QString& filepath, const QString& errorMessage, const QString& command);
  void onFinishedGeneratingReport();

 private:
  void coverPage();
  void tableOfContentsPage();

  bool classified;
  int* num_file;
  int* tot_num_files;
  int x = 325;
  int y = 400;
  std::string time;
  QPdfWriter* pdfWriter;
  QPainter* painter;
  Library* library;
  Model* model;
  Ui::ReportGenerationWindow* ui;
  ReportGeneratorWorker* reporterWorker;
  QThread* generatingReportThread;
  std::string subtitle;
  std::string label;
  std::string output_directory;
  std::string logo1_filepath;
  std::string logo2_filepath;
  std::string title;
  std::string username;
  std::string version;
  std::vector<std::string>* err_vec;
};

#endif  // REPORTGENERATIONWINDOW_H
