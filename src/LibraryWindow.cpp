
#include "./LibraryWindow.h"


LibraryWindow::LibraryWindow(QWidget* parent) : QWidget(parent)
{
  this->setFixedSize(QSize(640, 480));
  ui.setupUi(this);
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

  auto populateList = [](QListWidget* listWidget, const std::vector<std::string>& items) {
    /* clear existing data */
    listWidget->clear();
    for (const auto& item : items) {
      listWidget->addItem(QString::fromStdString(item));
    }
  };

  populateList(ui.geometryListView, library->getGeometry());
  populateList(ui.imagesListView, library->getImages());
  populateList(ui.documentsListView, library->getDocuments());
  populateList(ui.dataListView, library->getData());
}


void
LibraryWindow::on_allLibraries_clicked()
{
  this->hide();
}
