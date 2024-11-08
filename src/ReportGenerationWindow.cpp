#include "ReportGenerationWindow.h"

#include <QComboBox>
#include <QDesktopServices>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMessageBox>
#include <QPageLayout>
#include <QPushButton>
#include <QString>
#include <QThread>
#include <QUrl>
#include <QVBoxLayout>
#include <iostream>
#include <string>

#include "ReportGeneratorWorker.h"
#include "config.h"
#include "ui_reportgenerationwindow.h"

ReportGenerationWindow::ReportGenerationWindow(QWidget* parent, Model* model,
                                               Library* library)
    : QWidget(parent),
      model(model),
      library(library),
      num_file(0),
      ui(new Ui::ReportGenerationWindow),
      title("3D Model Inventory Report"),
      username("username"),
      version("Generated by CADventory 1.0.4 and BRLCAD 7.38.0") {
  ui->setupUi(this);

  connect(ui->outputDirectory_pushButton, &QPushButton::clicked, this,
          &ReportGenerationWindow::onOutputDirectoryButtonClicked);
  connect(ui->logo1_pushButton, &QPushButton::clicked, this,
          &ReportGenerationWindow::onLogo1ButtonClicked);
  connect(ui->logo2_pushButton, &QPushButton::clicked, this,
          &ReportGenerationWindow::onLogo2ButtonClicked);
  connect(ui->logo2_pushButton, &QPushButton::clicked, this,
          &ReportGenerationWindow::onLogo2ButtonClicked);
  connect(ui->generateReport_pushButton, &QPushButton::clicked, this,
          &ReportGenerationWindow::onGenerateReportButtonClicked);
}

void ReportGenerationWindow::onGenerateReportButtonClicked() {
  // need output directory
  if (output_directory.empty()) {
    QMessageBox msgBox(this);
    msgBox.setText("No Directory Found");
    QString message = "Please choose a directory to store your report.";
    msgBox.setInformativeText(message);
    msgBox.setStyleSheet("QLabel{min-width: 300px;}");
    msgBox.exec();
    return;
  }

  std::vector<std::string> existing_working_files;

  // check for any model.working files!
  for (const auto& model : model->getSelectedModels()) {
    std::string model_working_path =
        model.file_path.substr(0, model.file_path.find(".g")) + ".working";
    std::string model_working_name =
        model_working_path.substr(model_working_path.find_last_of("/\\") + 1);
    QDir dir_temp(QString::fromStdString(model_working_path));
    if (dir_temp.exists()) {
      // move it
      existing_working_files.push_back(model_working_path);
    }
  }
  if (existing_working_files.size()) {
    QString message;
    for (const auto& str : existing_working_files) {
      message += QString::fromStdString(str) +
                 "\n";  // Append each string with a newline
    }

    // Create and show the popup
    QMessageBox msgBox(this);
    msgBox.setText(
        "Found Model.working folders, please move or remove these folders "
        "before generating report.");
    msgBox.setInformativeText(message);
    msgBox.exec();
    return;
  }

  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  std::ostringstream oss;
  oss << std::put_time(&tm, "%m-%d-%Y, %H:%M:%S");
  time = oss.str();

  coverPage();
  tableOfContentsPage();

  err_vec = new std::vector<std::string>();

  num_file = new int(0);
  tot_num_files = new int(model->getSelectedModels().size());

  QThread* generatingReportThread =
      new QThread(this);  // Parent is ReportGenerationWindow
  ReportGeneratorWorker* reporterWorker =
      new ReportGeneratorWorker(model, output_directory, nullptr);
  // // Move the worker to the thread
  reporterWorker->moveToThread(generatingReportThread);

  // // Connect signals and slots
  connect(generatingReportThread, &QThread::started, reporterWorker,
          &ReportGeneratorWorker::process);
  connect(reporterWorker, &ReportGeneratorWorker::successfulGistCall, this,
          &ReportGenerationWindow::onSuccessfulGistCall);
  connect(reporterWorker, &ReportGeneratorWorker::failedGistCall, this,
          &ReportGenerationWindow::onFailedGistCall);
  connect(reporterWorker, &ReportGeneratorWorker::finishedReport, this,
          &ReportGenerationWindow::onFinishedGeneratingReport);
  connect(reporterWorker, &ReportGeneratorWorker::processingGistCall, this,
          &ReportGenerationWindow::onProcessingGistCall);

  // Start the indexing thread
  generatingReportThread->start();
}

void ReportGenerationWindow::coverPage() {
  /* portrait mode

  std::cout << "directory to save: " << output_directory << std::endl;
  std::string report_filepath = output_directory + "/report_" + time + ".pdf";

  pdfWriter = new QPdfWriter(QString::fromStdString(report_filepath));
  pdfWriter->setResolution(300);
  pdfWriter->setPageSize(QPageSize(QPageSize::A4));
  painter = new QPainter(pdfWriter);

  // fonts
  QFont font("Helvetica", 18);
  QFont font_two("Helvetica", 6);
  QFont title_font("Arial", 32);
  title_font.setWeight(QFont::Bold);
  QFont version_font("Arial", 12);
  QFont subtext_and_user_font("Arial", 12);
  int max_width = 2480;
  int max_height = 3508;

  // cover report

  // title
  if (!ui->title_textEdit->toPlainText().isEmpty()) {
    title = ui->title_textEdit->toPlainText().toStdString();
  }

  painter->setFont(title_font);
  painter->drawText(max_width / 2 - 900, max_height / 2 - 150,
                    QString::fromStdString(title));

  // top logo
  if (!logo1_filepath.empty()) {
    QPixmap top_logo(QString::fromStdString(logo1_filepath));
    painter->drawPixmap(300, 300, top_logo);
  }

  // version text
  if (!ui->version_textEdit->toPlainText().isEmpty()) {
    version = ui->version_textEdit->toPlainText().toStdString();
  }
  painter->setPen(Qt::gray);
  painter->setFont(version_font);

  // bottom logo, and placing version
  if (!logo2_filepath.empty()) {
    QPixmap bottom_logo(QString::fromStdString(logo2_filepath));
    painter->drawPixmap(max_width - 300 - bottom_logo.width(),
                        max_height - 300 - bottom_logo.height(), bottom_logo);
    painter->drawText(300, max_height - 300 - (bottom_logo.height() / 2),
                      QString::fromStdString(version));

  } else {
    painter->drawText(300, max_height - 300, QString::fromStdString(version));
  }

  // user and date, drawtext with rectangles
  if (!ui->username_textEdit->toPlainText().isEmpty()) {
    username = ui->username_textEdit->toPlainText().toStdString();
  }
  painter->setPen(Qt::black);
  painter->setFont(subtext_and_user_font);
  const QRect user_rect = QRect(max_width - 300 - 300, 300, 300, 150);
  painter->drawText(user_rect, Qt::AlignRight | Qt::TextWordWrap,
                    QString::fromStdString(username));
  const QRect date_rect = QRect(max_width - 300 - 600, 450, 600, 200);
  painter->drawText(date_rect, Qt::AlignRight,
                    QString::fromStdString(time.substr(0, time.find(","))));
  */
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  std::ostringstream oss;
  oss << std::put_time(&tm, "%m-%d-%Y, %H:%M:%S");
  auto time = oss.str();

  // std::cout << "directory to save: " << output_directory << std::endl;
  std::string report_filepath = output_directory + "/report_" + time + ".pdf";

  pdfWriter = new QPdfWriter(QString::fromStdString(report_filepath));
  pdfWriter->setResolution(300);
  pdfWriter->setPageSize(QPageSize(QPageSize::A4));
  pdfWriter->setPageOrientation(QPageLayout::Landscape);
  painter = new QPainter(pdfWriter);

  // // fonts
  QFont font("Helvetica", 18);
  QFont font_two("Helvetica", 6);
  QFont title_font("Arial", 32);
  title_font.setWeight(QFont::Bold);
  QFont version_font("Arial", 12);
  QFont subtext_and_user_font("Arial", 12);
  int Margin = 300;

  // cover report
  QPixmap top_logo;
  // // top logo
  if (!logo1_filepath.empty()) {
    top_logo = QPixmap(QString::fromStdString(logo1_filepath));
    painter->drawPixmap(Margin, Margin, top_logo);
  }

  // bottom logo, and placing version
  if (!logo2_filepath.empty()) {
    QPixmap bottom_logo(QString::fromStdString(logo2_filepath));
    painter->drawPixmap(A4_MAXWIDTH_LS - Margin - bottom_logo.width(),
                        A4_MAXHEIGHT_LS - Margin - bottom_logo.height(),
                        bottom_logo);
  } else {
    // TODO: draw brlcad logo from qt resource system/built in :)
    std::cout << "default logo!!" << std::endl;
    QPixmap bottom_logo(":/src/assets/brlcad_logo.png");
    painter->drawPixmap(A4_MAXWIDTH_LS - Margin - bottom_logo.width(),
                        A4_MAXHEIGHT_LS - Margin - bottom_logo.height(),
                        bottom_logo);
  }
  painter->drawText(Margin, A4_MAXHEIGHT_LS - Margin,
                    QString::fromStdString(version));

  // title
  if (!ui->title_textEdit->toPlainText().isEmpty()) {
    title = ui->title_textEdit->toPlainText().toStdString();
  }

  // what is default margin
  painter->setPen(QPen(Qt::black, 3));
  painter->setFont(title_font);
  // const QRect zero_rect = QRect(Margin, Margin, 100, 100);

  int mx = Margin + 500;
  int my = Margin + 500;  // assuming logo heights are <= 500 pixels
  QRect title_rect(mx, my, A4_MAXWIDTH_LS - 2 * mx, A4_MAXHEIGHT_LS - 2 * my);
  painter->drawText(title_rect,
                    Qt::AlignVCenter | Qt::AlignHCenter | Qt::TextWordWrap,
                    QString::fromStdString(title));
  // // user and date, drawtext with rectangles
  if (!ui->username_textEdit->toPlainText().isEmpty()) {
    username = ui->username_textEdit->toPlainText().toStdString();
  }
  painter->setPen(Qt::black);
  painter->setFont(subtext_and_user_font);
  const QRect user_rect = QRect(A4_MAXWIDTH_LS - 300 - 500, 300, 500, 100);
  painter->drawText(user_rect, Qt::AlignRight | Qt::TextWordWrap,
                    QString::fromStdString(username));
  const QRect date_rect = QRect(A4_MAXWIDTH_LS - 300 - 500, 400, 500, 100);
  painter->drawText(date_rect, Qt::AlignRight,
                    QString::fromStdString(time.substr(0, time.find(","))));
}

void ReportGenerationWindow::tableOfContentsPage() {
  /* portrait mode
  QFont font("Helvetica", 18);
  QFont font_two("Helvetica", 6);
  if (pdfWriter->newPage()) {
    painter->setFont(font);
    painter->drawText(750, 200, "Cadventory");
    painter->setPen(QPen(Qt::black, 3));
    painter->drawLine(-200, 250, 2450, 250);
    painter->drawLine(-200, 3400, 2450, 3400);
    painter->setFont(font_two);
    painter->drawText(
        300, 300,
        QString::fromStdString("Library: " + std::string(library->name())));
    painter->drawText(300, 350,
                      QString::fromStdString("Report Generated on: " + time));
    // will need to adjust for windows, macos "local home" libraries
  } else {
    std::cout << "error: new page failed" << std::endl;
  }

  painter->drawText(x, y, "Geometry");
  y += 25;

  for (const auto& str : library->getGeometry()) {
    y += 25;
    painter->drawText(x, y, QString::fromStdString(str));
  }
  y += 50;
  painter->drawText(x, y, "Images");
  y += 25;
  for (const auto& str : library->getImages()) {
    y += 25;
    painter->drawText(x, y, QString::fromStdString(str));
  }
  y += 50;
  painter->drawText(x, y, "Documents");
  y += 25;
  for (const auto& str : library->getDocuments()) {
    y += 25;
    painter->drawText(x, y, QString::fromStdString(str));
  }

  ProcessGFiles gFileProcessor(model);
  std::cout << "generating gist reports" << std::endl;
  painter->setFont(font);
  painter->rotate(90);
  */

  // width = 600
  // height = 100
  // draw "Table of Contents" in middle of page
  int x_tr = int(A4_MAXWIDTH_LS / 2) - 600;
  int y_tr = 300;
  int height_tr = 130;
  int width_tr = 1200;
  QRect title_rect(x_tr, y_tr, width_tr, height_tr);

  // draw "table"
  int x_tb = 300;
  int y_tb = height_tr + y_tr + 150;
  int width_tb = A4_MAXWIDTH_LS - (2 * 300);
  int height_tb =
      A4_MAXHEIGHT_LS - (2 * 300) -
      height_tr;  // last 300 is margin in between table of contents and table
  QRect table_rect(x_tb, y_tb, width_tb, height_tb);

  QFont table_header_font("Arial", 12);
  QFont table_font("Arial", 6);
  QFont title_font("Arial", 32);

  // x positions for each column
  int num_item_x = x_tb;
  int model_name_x = num_item_x + 200;
  int primary_comp_x = model_name_x + 600;
  int parent_dir_x = primary_comp_x + 600;
  int tag_rect_x = parent_dir_x + 600;
  int row_y = y_tb;

  // row height  = 50px
  // 33 models per page of table of contents

  // draw header row in tbale
  QRect num_item_rect = QRect(num_item_x, row_y, 200, 100);
  QRect model_name_rect = QRect(model_name_x, row_y, 600, 100);
  QRect primary_comp_rect = QRect(primary_comp_x, row_y, 600, 100);
  QRect parent_dir_rect = QRect(parent_dir_x, row_y, 600, 100);
  QRect tag_rect = QRect(tag_rect_x, row_y, 908, 100);

  // every 33 models new page
  int model_count = 0;
  for (const auto& modelData : model->getSelectedModels()) {
    if (model_count % 33 == 0) {
      row_y = y_tb;
      if (pdfWriter->newPage()) {
        // draw title
        painter->setPen(QPen(Qt::black, 3));
        painter->setFont(title_font);
        painter->drawText(title_rect, Qt::AlignVCenter | Qt::AlignHCenter,
                          QString::fromStdString("Table of Contents"));

        // draw table
        painter->drawRect(table_rect);

        // draw table header
        painter->setPen(QPen(Qt::black, 1));
        painter->setFont(table_header_font);
        num_item_rect = QRect(num_item_x, row_y, 200, 100);
        model_name_rect = QRect(model_name_x, row_y, 600, 100);
        primary_comp_rect = QRect(primary_comp_x, row_y, 600, 100);
        parent_dir_rect = QRect(parent_dir_x, row_y, 600, 100);
        tag_rect = QRect(tag_rect_x, row_y, 908, 100);
        painter->drawRect(num_item_rect);
        painter->drawRect(model_name_rect);
        painter->drawRect(primary_comp_rect);
        painter->drawRect(parent_dir_rect);
        painter->drawRect(tag_rect);

        painter->drawText(num_item_rect, Qt::AlignVCenter | Qt::AlignHCenter,
                          QString::fromStdString("#"));
        painter->drawText(model_name_rect, Qt::AlignVCenter | Qt::AlignHCenter,
                          QString::fromStdString("model_name"));
        painter->drawText(primary_comp_rect,
                          Qt::AlignVCenter | Qt::AlignHCenter,
                          QString::fromStdString("primary component"));
        painter->drawText(parent_dir_rect, Qt::AlignVCenter | Qt::AlignHCenter,
                          QString::fromStdString("parent directory"));
        painter->drawText(tag_rect, Qt::AlignVCenter | Qt::AlignHCenter,
                          QString::fromStdString("tags"));
        row_y += 100;
      } else {
        std::cerr << "Error in creating new page" << std::endl;
      }
    }

    // get primary object
    std::string primary_obj = "";
    std::vector<ObjectData> associatedObjects =
        model->getObjectsForModel(modelData.id);

    if (associatedObjects.empty()) {
      std::cout << "No associated objects for this model.\n";
    } else {
      std::cout << "Associated Objects (" << associatedObjects.size() << "):\n";
      for (const auto& obj : associatedObjects) {
        if (obj.is_selected) {
          primary_obj = obj.name;
        }
      }
    }
    // draw model row
    painter->setFont(table_font);
    num_item_rect = QRect(num_item_x, row_y, 200, 50);
    model_name_rect = QRect(model_name_x, row_y, 600, 50);
    primary_comp_rect = QRect(primary_comp_x, row_y, 600, 50);
    parent_dir_rect = QRect(parent_dir_x, row_y, 600, 50);
    tag_rect = QRect(tag_rect_x, row_y, 908, 50);
    painter->drawRect(num_item_rect);
    painter->drawRect(model_name_rect);
    painter->drawRect(primary_comp_rect);
    painter->drawRect(parent_dir_rect);
    painter->drawRect(tag_rect);
    painter->drawText(num_item_rect, Qt::AlignVCenter | Qt::AlignHCenter,
                      QString::fromStdString(std::to_string(model_count)));
    painter->drawText(model_name_rect, Qt::AlignVCenter | Qt::AlignHCenter,
                      QString::fromStdString(modelData.short_name));
    painter->drawText(primary_comp_rect, Qt::AlignVCenter | Qt::AlignHCenter,
                      QString::fromStdString(primary_obj));
    painter->drawText(parent_dir_rect, Qt::AlignVCenter | Qt::AlignHCenter,
                      QString::fromStdString(library->fullPath));
    painter->drawText(tag_rect, Qt::AlignVCenter | Qt::AlignHCenter,
                      QString::fromStdString("tags"));
    row_y += 50;
    model_count++;
  }
}
void ReportGenerationWindow::onOutputDirectoryButtonClicked() {
  QString temp_dir_1 = QFileDialog::getExistingDirectory(
      this, tr("Choose Directory to store Output"));

  if (temp_dir_1.isEmpty()) {
    QMessageBox msgBox(this);
    msgBox.setText("No Directory Found");
    QString message = "Please choose a directory to store your report.";
    msgBox.setInformativeText(message);
    msgBox.setStyleSheet("QLabel{min-width: 300px;}");
    msgBox.exec();
    return;
  }
  ui->outputDirectory_textEdit->setPlainText(temp_dir_1);
  output_directory = temp_dir_1.toStdString();
}
void ReportGenerationWindow::onLogo1ButtonClicked() {
  QString top_logo_path =
      QFileDialog::getOpenFileName(this, tr("Choose first logo for report."),
                                   "", tr("Images (*.png *.xpm *.jpg)"));
  ui->logo1_textEdit->setPlainText(top_logo_path);
  logo1_filepath = top_logo_path.toStdString();
}
void ReportGenerationWindow::onLogo2ButtonClicked() {
  QString bottom_logo_path =
      QFileDialog::getOpenFileName(this, tr("Choose second logo for report."),
                                   "", tr("Images (*.png *.xpm *.jpg)"));
  ui->logo2_textEdit->setPlainText(bottom_logo_path);
  logo2_filepath = bottom_logo_path.toStdString();
}

void ReportGenerationWindow::onProcessingGistCall(const QString& file) {
  std::string cur_file =
      "Processing: " +
      file.toStdString().substr(file.toStdString().find_last_of("/\\") + 1);
  ui->fileInProcess_label->setText(QString::fromStdString(cur_file));
}

void ReportGenerationWindow::onSuccessfulGistCall(
    const QString& path_gist_output) {
  // Load the generated PNG image
  QPixmap gist(path_gist_output);

  // bool status_newpage = pdfWriter->newPage();
  if (pdfWriter->newPage()) {
    painter->drawPixmap(0, 0, gist);
  } else {
    std::cerr << "Failed to create new PDF page. (onSuccessfulGistCall)"
              << std::endl;
  }

  int progress = (*num_file) * 100 / (*tot_num_files);
  ui->progressBar->setValue(progress);
  (*num_file)++;
}
void ReportGenerationWindow::onFailedGistCall(const QString& filepath,
                                              const QString& errorMessage) {
  std::string err = "Model: " + filepath.toStdString() + "\nError:\n" +
                    errorMessage.toStdString();
  QFont font("Helvetica", 18);
  QFont font_two("Helvetica", 6);
  err_vec->push_back(err);
  if (pdfWriter->newPage()) {
    painter->rotate(-90);
    painter->setFont(font);
    painter->drawText(100, 100, QString::fromStdString(filepath.toStdString()));
    painter->setFont(font_two);
    painter->drawText(100, 150,
                      QString::fromStdString(errorMessage.toStdString()));
    painter->rotate(90);
  } else {
    std::cerr << "Failed to create new PDF page. (onFailedGistCall)"
              << std::endl;
  }
  int progress = (*num_file) * 100 / (*tot_num_files);
  ui->progressBar->setValue(progress);
  (*num_file)++;
}

void ReportGenerationWindow::onFinishedGeneratingReport() {
  ui->progressBar->setValue(100);
  ui->fileInProcess_label->setText(QString::fromStdString("Complete"));

  painter->end();

  std::vector<ModelData> selectedModels = model->getSelectedModels();

  std::string hidden_dir_path = library->fullPath + "/.cadventory";

  // yay this works

  // make a temp dir for this report!
  std::string working_folders_dir =
      hidden_dir_path + "/working_folders_" + time;
  QDir wfdir(QString::fromStdString(working_folders_dir));
  if (!wfdir.exists()) {
    wfdir.mkpath(".");
  } else {
    std::cout << "folder exists!" << std::endl;
  }

  for (const auto& model : selectedModels) {
    std::string model_working_path =
        model.file_path.substr(0, model.file_path.find(".g")) + ".working";
    std::string model_working_name =
        model_working_path.substr(model_working_path.find_last_of("/\\") + 1);
    QDir dir_temp(QString::fromStdString(model_working_path));
    if (dir_temp.exists()) {
      // move it
      std::string dest = working_folders_dir + "/" + model_working_name;
      QDir dir_temp_two;
      if (!dir_temp_two.rename(QString::fromStdString(model_working_path),
                               QString::fromStdString(dest))) {
        std::cerr
            << "Moving model.working file failed (onFinishedGeneratingReport)"
            << std::endl;
      }
    }
  }

  std::cout << "Report Generated" << std::endl;
  if (err_vec->size()) {
    // Convert std::vector<std::string> to QString
    QString message;
    for (const auto& str : *err_vec) {
      message += QString::fromStdString(str) +
                 "\n";  // Append each string with a newline
    }

    // Create and show the popup
    QMessageBox msgBox(this);
    msgBox.setText("Errors:");
    msgBox.setInformativeText(message);
    msgBox.exec();
  }
  QDesktopServices::openUrl(QUrl(
      QString::fromStdString(output_directory)));  // open pdf after generation
}

ReportGenerationWindow::~ReportGenerationWindow() {
  delete ui;
  if (pdfWriter) {
    delete pdfWriter;
  }
  if (painter) {
    delete painter;
  }
  if (err_vec) {
    delete err_vec;
  }
  if (num_file) {
    delete num_file;
  }
  if (tot_num_files) {
    delete tot_num_files;
  }
}
