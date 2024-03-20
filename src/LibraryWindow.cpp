
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

  /* populate the Models listing */
  std::vector<std::string> modelDirs = library->getModels();
  ui.listWidget->clear();

  /* add each dir to the list */
  for (const std::string& dir : modelDirs) {
    QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(dir));
    ui.listWidget->addItem(item);
  }
}


void
LibraryWindow::on_allLibraries_clicked()
{
  this->hide();
}
