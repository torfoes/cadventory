
#include "./LibraryWindow.h"
#include "./Model.h"

#include <QStringListModel>
#include <QListWidgetItem>

#include <iostream>


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


void LibraryWindow::loadFromLibrary(Library* _library)
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

  /* prepare MVC x for our list views */
  auto populateModel = [this](QStringListModel* listModel, const std::vector<std::string>& fullPaths) {
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
  
  ui.tagsListView->setModel(tagsModel);
  ui.currentTagsListView->setModel(currentTagsModel);

  /* load tags */
  std::cout << "Calling display tags for library: " << library->name() << std::endl;
  loadTags();
  std::cout << "Called display tags for library: " << library->name() << std::endl;
}


void LibraryWindow::on_allLibraries_clicked()
{
  this->hide();
}

void LibraryWindow::updateListModelForDirectory(QStringListModel* listModel, const std::vector<std::string>& allItems, const std::string& directory)
{
  QStringList filteredItems;
  std::string base = library->path() + (directory == "." ? "" : "/" + directory);
  for (const auto& item : allItems) {
    if (item.find(base) == 0) { // item starts with base path
      std::string relativePath = item.substr(base.length() + 1); // +1 to skip leading slash
      filteredItems << QString::fromStdString(relativePath);
    }
  }
  listModel->setStringList(filteredItems);
}


void LibraryWindow::onModelSelectionChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
  if (!current)
    return; // nada selected

  QString selectedDir = current->text();
  int modelId = library->model->hashModel(library->fullPath+"/"+selectedDir.toStdString());
  std::vector<std::string> modelTags = library->model->getTagsForModel(modelId);

  std::cout << "Selected model: " << selectedDir.toStdString() << std::endl;
  std::cout << "Tags for model: " << std::endl;
  for (const auto& tag : modelTags) {
    std::cout << tag << " ";
  }
  std::cout << std::endl;

  QStringList qModelTags;
  for (const auto& tag : modelTags) {
    qModelTags << QString::fromStdString(tag);
  }
  currentTagsModel->setStringList(qModelTags);

  /* retrieve full lists */
  // std::vector<std::string> allGeometry = library->getGeometry();
  // std::vector<std::string> allImages = library->getImages();
  // std::vector<std::string> allDocuments = library->getDocuments();
  // std::vector<std::string> allData = library->getData();

  /* filter and update the models for each category */
  // std::cout << "Updating models for directory: " << selectedDir.toStdString() << std::endl;
  // updateListModelForDirectory(geometryModel, allGeometry, selectedDir.toStdString());
  // std::cout << "Updated geometry model" << std::endl;
  // updateListModelForDirectory(imagesModel, allImages, selectedDir.toStdString());
  // std::cout << "Updated images model" << std::endl;
  // updateListModelForDirectory(documentsModel, allDocuments, selectedDir.toStdString());
  // std::cout << "Updated documents model" << std::endl;
  // updateListModelForDirectory(dataModel, allData, selectedDir.toStdString());
  // std::cout << "Updated data model" << std::endl;
}

// loads all tags from models from a directory ordered by prevalence
void LibraryWindow::loadTags()
{
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

void displayModel(const ModelData& model)
{
  std::cout << "Model: " << model.short_name << std::endl;
  std::cout << "Primary file: " << model.primary_file << std::endl;
  std::cout << "Override info: " << model.override_info << std::endl;
  std::cout << "Properties: " << std::endl;
  for (const auto& [key, value] : model.properties) {
    std::cout << "  " << key << ": " << value << std::endl;
  }


}