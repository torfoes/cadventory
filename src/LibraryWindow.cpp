
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
}


void
LibraryWindow::on_allLibraries_clicked()
{
  this->hide();
}
