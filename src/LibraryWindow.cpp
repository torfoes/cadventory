#include "ProcessGFiles.h"
#include <filesystem>
#include "./LibraryWindow.h"
#include <iostream>
#include <QStringListModel>


LibraryWindow::LibraryWindow(QWidget* parent) : QWidget(parent)
{
  this->setFixedSize(QSize(640, 480));
  ui.setupUi(this);

  /* make Model selections update the tabs */
  connect(ui.listWidget, &QListWidget::currentItemChanged, this, &LibraryWindow::onModelSelectionChanged);

}


LibraryWindow::~LibraryWindow()
{
}


void
LibraryWindow::loadFromLibrary(Library* _library)
{
  this->library = _library;
  this->setWindowTitle(library->name() + QString(" Library"));
  this->ui.currentLibrary->setText(library->name());

  /* start fresh */
  ui.listWidget->clear();

  /* populate Models listing */
  auto modelDirs = library->getModels();
  for (const auto& dir : modelDirs) {
    QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(dir));
    ui.listWidget->addItem(item);
  }

  /* prepare MVC model for our list views */
  auto populateModel = [this](QStringListModel* model, const std::vector<std::string>& fullPaths) {
    QStringList list;
    std::string base = std::string(library->path()) + "/";
    for (const auto& fullPath : fullPaths) {
      std::string relativePath = fullPath.substr(base.length());
      list << QString::fromStdString(relativePath);
    }
    model->setStringList(list);
  };

  geometryModel = new QStringListModel(this);
  imagesModel = new QStringListModel(this);
  documentsModel = new QStringListModel(this);
  dataModel = new QStringListModel(this);

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
}


void
LibraryWindow::on_allLibraries_clicked()
{
  this->hide();
}


void
LibraryWindow::updateListModelForDirectory(QStringListModel* model, const std::vector<std::string>& allItems, const std::string& directory)
{
  QStringList filteredItems;
  std::string base = library->path() + (directory == "." ? "" : "/" + directory);
  for (const auto& item : allItems) {
    if (item.find(base) == 0) { // item starts with base path
      std::string relativePath = item.substr(base.length() + 1); // +1 to skip leading slash
      filteredItems << QString::fromStdString(relativePath);
    }
  }
  model->setStringList(filteredItems);
}


void
LibraryWindow::onModelSelectionChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
  if (!current)
    return; // nada selected

  QString selectedDir = current->text();

  /* retrieve full lists */
  std::vector<std::string> allGeometry = library->getGeometry();
  std::vector<std::string> allImages = library->getImages();
  std::vector<std::string> allDocuments = library->getDocuments();
  std::vector<std::string> allData = library->getData();

  // Instantiate the ProcessGFiles class
  std::string databasePath = "cadventory.db";  // Database path, adjust if needed
  ProcessGFiles gFileProcessor(databasePath);

  // Iterate over each geometry file
  for (const auto& file : allGeometry) {
      std::filesystem::path filePath(file);
      if (filePath.extension() == ".g") {
          // Process the .g file if it's a valid BRL-CAD geometry file
          gFileProcessor.processGFile(filePath);
          std::cout << "Processed .g file: " << filePath << std::endl;
      } else {
          std::cout << "Skipped non-.g file: " << filePath << std::endl;
      }
  }

  /* filter and update the models for each category */
  updateListModelForDirectory(geometryModel, allGeometry, selectedDir.toStdString());
  updateListModelForDirectory(imagesModel, allImages, selectedDir.toStdString());
  updateListModelForDirectory(documentsModel, allDocuments, selectedDir.toStdString());
  updateListModelForDirectory(dataModel, allData, selectedDir.toStdString());
}


