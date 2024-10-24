#include "./LibraryWindow.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPainter>
#include <QPdfWriter>
#include <QPixmap>
#include <QScreen>
#include <QString>
#include <QStringList>
#include <QStringListModel>
#include <QStyledItemDelegate>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "./Model.h"
#include "./ProcessGFiles.h"

namespace fs = std::filesystem;

LibraryWindow::LibraryWindow(QWidget* parent) : QWidget(parent) {
  this->setFixedSize(QSize(876, 600));
  ui.setupUi(this);

  // tagsWidget = new QListWidget(this);

  /* make Model selections update the tabs */
  connect(ui.listWidget, &QListWidget::currentItemChanged, this,
          &LibraryWindow::onModelSelectionChanged);
  connect(ui.generateReport, &QPushButton::pressed, this,
          &LibraryWindow::generateReport);
}

LibraryWindow::~LibraryWindow() {}

void LibraryWindow::generateReport() {
  // get time
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  std::ostringstream oss;
  oss << std::put_time(&tm, "%m-%d-%Y, %H:%M:%S");
  auto time = oss.str();
  QString temp_dir_1 = QFileDialog::getExistingDirectory(
      this, tr("Choose Directory to store Output"));

  if (temp_dir_1.toStdString().length() != 0) {
    std::cout << "directory to save: " << temp_dir_1.toStdString() << std::endl;
    std::string report_filepath =
        temp_dir_1.toStdString() + "/report_" + time + ".pdf";
    QPdfWriter pdfWriter(QString::fromStdString(report_filepath));

    // Set the resolution (optional)
    pdfWriter.setResolution(300);

    // Set the page size (A4, Letter, etc.)
    pdfWriter.setPageSize(QPageSize(QPageSize::A4));

    // Create a QPainter to draw on the QPdfWriter
    QPainter painter(&pdfWriter);

    // Set a font for the text
    QFont font("Helvetica", 18);
    painter.setFont(font);
    painter.drawText(750, 200, "Cadventory");
    painter.setPen(QPen(Qt::black, 3));
    painter.drawLine(-200, 250, 2450, 250);
    painter.drawLine(-200, 3400, 2450, 3400);
    QFont font_two("Helvetica", 6);
    painter.setFont(font_two);
    painter.drawText(
        300, 300,
        QString::fromStdString("Library: " + std::string(library->name())));
    painter.drawText(300, 350,
                     QString::fromStdString("Report Generated on: " + time));
    // will need to adjust for windows, macos "local home" libraries

    std::string dir_slash = std::string(library->name()) + "/";
    int x = 325;
    int y = 400;
    painter.drawText(x, y, "Geometry");

    y += 25;

    for (const auto& str : geometryModel->stringList()) {
      std::string cur_model = str.toStdString();
      y += 25;
      painter.drawText(x, y, QString::fromStdString(cur_model));
    }
    y += 50;
    painter.drawText(x, y, "Images");
    y += 25;
    for (const auto& str : imagesModel->stringList()) {
      std::string cur_model = str.toStdString();
      y += 25;
      painter.drawText(x, y, QString::fromStdString(cur_model));
    }
    y += 50;
    painter.drawText(x, y, "Documents");
    y += 25;
    for (const auto& str : documentsModel->stringList()) {
      std::string cur_model = str.toStdString();
      y += 25;
      painter.drawText(x, y, QString::fromStdString(cur_model));
    }

    ProcessGFiles gFileProcessor;  // just to call commands
    int num_file = 0;
    std::cout << "generating gist reports" << std::endl;
    painter.setFont(font);
    painter.rotate(90);

    std::vector<std::string> err_vec;
    for (const auto& modelData : report) {
      std::string fileName = modelData.short_name;
      std::string str = library->fullPath + "/" + modelData.path;

      std::cout << "model selected: " << fileName << std::endl;

      std::string path_gist_output =
          temp_dir_1.toStdString() + "/" + std::to_string(num_file) + ".png";
      std::string gist_command = "/home/anton/brlcad/build/bin/gist " + str +
                                 " -o " + path_gist_output;

      auto [output, error] = gFileProcessor.runCommand(gist_command);
      // End painting
      std::cout << "std output: " << output << std::endl;
      std::string png =
          temp_dir_1.toStdString() + "/" + std::to_string(num_file) + ".png";
      QString png_qstr = QString::fromStdString(png);
      QPixmap gist(png_qstr);
      bool status_newpage = pdfWriter.newPage();
      if (output.find("ERROR") == std::string::npos) {
        painter.drawPixmap(0, -2408, gist);
      } else {
        std::string err = "model: " + fileName + "\nerror:\n" + output +
                          "\ncommand: \n" + gist_command;
        err_vec.push_back(err);
        painter.rotate(-90);
        painter.setFont(font);
        painter.drawText(100, 100, QString::fromStdString(str));
        painter.setFont(font_two);
        painter.drawText(100, 150, QString::fromStdString(output));
        painter.rotate(90);
      }
      num_file++;
    }

    painter.end();

    std::cout << "Report Generated" << std::endl;
    if (err_vec.size()) {
      // Convert std::vector<std::string> to QString
      QString message;
      for (const auto& str : err_vec) {
        message += QString::fromStdString(str) +
                   "\n";  // Append each string with a newline
      }

      // Create and show the popup
      QMessageBox msgBox(this);
      msgBox.setText("Errors:");
      msgBox.setInformativeText(message);
      msgBox.exec();
    }
    QDesktopServices::openUrl(QUrl(temp_dir_1));  // open pdf after generation

  } else {
    QMessageBox msgBox(this);
    msgBox.setText("No Directory Found");
    QString message = "Please choose a directory to store your report.";
    msgBox.setInformativeText(message);
    msgBox.setStyleSheet("QLabel{min-width: 300px;}");

    msgBox.exec();
  }
}

void LibraryWindow::loadFromLibrary(Library* _library) {
  this->library = _library;
  this->setWindowTitle(library->name() + QString(" Library"));
  this->ui.currentLibrary->setText(library->name());

  /* start fresh */
  ui.listWidget->clear();

  /* populate Models listing */
  library->setModels();
  std::cout << "Model Dirs length: " << library->models.size() << std::endl;
  for (const auto& modelData : library->models) {
    QString displayName = QString::fromStdString(modelData.short_name);
    QListWidgetItem* item = new QListWidgetItem(displayName);
    // Optionally store model ID in the item for later use
    item->setData(Qt::UserRole, modelData.id);
    ui.listWidget->addItem(item);
  }

  /* prepare MVC x for our list views */
  auto populateModel = [this](QStringListModel* listModel,
                              const std::vector<std::string>& fullPaths) {
    QStringList list;
    std::string base = std::string(library->path()) + "/";
    for (const auto& fullPath : fullPaths) {
      std::string relativePath = fullPath.substr(base.length());
      list << QString::fromStdString(relativePath);
    }
    listModel->setStringList(list);
  };

  geometryModel = new QStringListModel(this);
  imagesModel = new QStringListModel(this);
  documentsModel = new QStringListModel(this);
  dataModel = new QStringListModel(this);
  tagsModel = new QStringListModel(this);
  currentTagsModel = new QStringListModel(this);
  currentPropertiesModel = new QStringListModel(this);

  /* populate the MVC models */
  populateModel(geometryModel, library->getGeometry());
  populateModel(imagesModel, library->getImages());
  populateModel(documentsModel, library->getDocuments());
  populateModel(dataModel, library->getData());

  /* wire the MVC models to the list views */
  ui.geometryListView->setModel(geometryModel);
  ui.imagesListView->setModel(imagesModel);
  ui.documentsListView->setModel(documentsModel);
  ui.dataListView->setModel(dataModel);

  std::cout << "Setting tags model" << std::endl;
  ui.tagsListView->setModel(tagsModel);
  std::cout << "Seting current tags model" << std::endl;
  ui.currentTagsListView->setModel(currentTagsModel);
  std::cout << "Setting current properties model" << std::endl;
  ui.currentPropertiesListView->setModel(currentPropertiesModel);
  std::cout << "Set current properties model" << std::endl;

  /* load tags */
  std::cout << "Calling display tags for library: " << library->name()
            << std::endl;
  loadTags();
  std::cout << "Called display tags for library: " << library->name()
            << std::endl;

  std::string root_folder =
      fs::current_path().string() +
      "/previews/";  // Get the current root folder of the project

  for (const QString& path : geometryModel->stringList()) {
    // Check if the string ends with ".g"
    if (path.endsWith(".g", Qt::CaseInsensitive)) {
      QString filename = QFileInfo(path).fileName();
      // std::cout << filename.toStdString() << std::endl;
      // QString filepath = ":/build/previews/"+filename;

      QString filepath = QString::fromStdString(root_folder);

      filepath.append(filename);
      filepath.chop(2);
      filepath.append(".png");
      // std::cout << filepath.toStdString() << std::endl;
      // Print the filename without the extension

      QListWidgetItem* item = new QListWidgetItem(QIcon(filepath), filename);
      QList<QSize> availableSizes = item->icon().availableSizes();

      if (availableSizes.isEmpty()) {
        delete item;
      } else {
        ui.listWidgetPage->addItem(item);
      }
    }
  }

  ui.listWidgetPage->setResizeMode(QListView::Adjust);
  ui.listWidgetPage->setViewMode(QListView::IconMode);

  ui.listWidgetPage->setWordWrap(true);

  ui.listWidgetPage->setIconSize(QSize(72, 72));
  ui.listWidgetPage->setGridSize(QSize(144, 108));
  ui.listWidgetPage->setUniformItemSizes(true);
  ui.listWidgetPage->setMovement(QListView::Static);
  ui.listWidgetPage->setResizeMode(QListView::Adjust);
  ui.listWidgetPage->setLayoutMode(QListView::Batched);
  ui.listWidgetPage->setBatchSize(10);

  QStyledItemDelegate* delegate = new QStyledItemDelegate(this);
  ui.listWidgetPage->setItemDelegate(delegate);
}

void LibraryWindow::on_allLibraries_clicked() {
  this->hide();
  main->show();
}

void LibraryWindow::updateListModelForDirectory(
    QStringListModel* listModel, const std::vector<std::string>& allItems,
    const std::string& directory) {
  QStringList filteredItems;
  std::string base =
      library->path() + (directory == "." ? "" : "/" + directory);
  for (const auto& item : allItems) {
    if (item.find(base) == 0) {  // item starts with base path
      std::string relativePath =
          item.substr(base.length() + 1);  // +1 to skip leading slash
      filteredItems << QString::fromStdString(relativePath);
    }
  }
  listModel->setStringList(filteredItems);
}

void LibraryWindow::onModelSelectionChanged(QListWidgetItem* current,
                                            QListWidgetItem* /*previous*/) {
  if (!current) return;  // nada selected

  int modelId = current->data(Qt::UserRole).toInt();

  std::vector<std::string> modelTags = library->model->getTagsForModel(modelId);
  std::cout << ">>Selected model name: "
            << library->model->getModelById(modelId).short_name << std::endl;

  QStringList qModelTags;
  for (const auto& tag : modelTags) {
    qModelTags << QString::fromStdString(tag);
  }
  currentTagsModel->setStringList(qModelTags);

  QStringList qModelProperties;
  std::map<std::string, std::string> modelProperties =
      library->model->getProperties(modelId);
  for (const auto& [key, value] : modelProperties) {
    qModelProperties << QString::fromStdString(key + ": " + value);
  }
  currentPropertiesModel->setStringList(qModelProperties);

  /* retrieve full lists */
  // std::vector<std::string> allGeometry = library->getGeometry();
  // std::vector<std::string> allImages = library->getImages();
  // std::vector<std::string> allDocuments = library->getDocuments();
  // std::vector<std::string> allData = library->getData();

  /* filter and update the models for each category */
  // std::cout << "Updating models for directory: " << selectedDir.toStdString()
  // << std::endl; updateListModelForDirectory(geometryModel, allGeometry,
  // selectedDir.toStdString()); std::cout << "Updated geometry model" <<
  // std::endl; updateListModelForDirectory(imagesModel, allImages,
  // selectedDir.toStdString()); std::cout << "Updated images model" <<
  // std::endl; updateListModelForDirectory(documentsModel, allDocuments,
  // selectedDir.toStdString()); std::cout << "Updated documents model" <<
  // std::endl; updateListModelForDirectory(dataModel, allData,
  // selectedDir.toStdString()); std::cout << "Updated data model" << std::endl;
}

// loads all tags from models from a directory ordered by prevalence
void LibraryWindow::loadTags() {
  std::cout << "Displaying tags for library: " << library->name() << std::endl;

  std::vector<std::string> sortedTags = library->getTags();
  for (const auto& tag : sortedTags) {
    std::cout << tag << " ";
  }
  std::cout << std::endl;
  QStringList tags;
  for (const auto& tag : sortedTags) {
    tags << QString::fromStdString(tag);
  }
  std::cout << "Setting tags model" << std::endl;
  tagsModel->setStringList(tags);
}

// void LibraryWindow::on_listWidgetPage_itemClicked(QListWidgetItem *item)
// {
//     ui.listWidgetPage->
// }

void LibraryWindow::on_pushButton_clicked() {
  report = this->library->getModels();
  for (auto& modelData : report) {
    modelData.path = library->fullPath + "/" + modelData.path;
  }
}

void LibraryWindow::on_listWidgetPage_itemClicked(QListWidgetItem* item) {
  std::string name = item->text().toStdString();

  auto it = std::find_if(
      report.begin(), report.end(),
      [&name](const ModelData& md) { return md.short_name == name; });

  if (it != report.end()) {
    report.erase(it);

    QFont font = item->font();
    font.setBold(false);
    item->setFont(font);
    std::cout << name + " is unselected from report" << std::endl;

  } else {
    auto allModels = library->getModels();
    auto modelIt = std::find_if(
        allModels.begin(), allModels.end(),
        [&name](const ModelData& md) { return md.short_name == name; });
    if (modelIt != allModels.end()) {
      report.push_back(*modelIt);

      QFont font = item->font();
      font.setBold(true);
      item->setFont(font);
      std::cout << name + " is selected for report" << std::endl;
    }
  }
}
