
#include "./MainWindow.h"

#include <iostream>
#include <QFileDialog>
#include <QPushButton>
#include <QSettings>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
  this->setFixedSize(QSize(876, 600));
  ui.setupUi(this);

  /* adjust the + button label position */
  QLayoutItem* item = ui.gridLayout->itemAt(0);
  QPushButton* addButton = qobject_cast<QPushButton*>(item->widget());
  if (addButton) {
    addButton->setStyleSheet("QPushButton { padding-top: -10px; }");
  }

  /* To manually reset on Mac:
   * defaults delete com.brl-cad.CADventory
   */
  size_t loaded = loadState();
  if (!loaded) {
    addLibrary("Local Home", QDir::homePath().toStdString().c_str());
  } else {
    std::cout << "Loaded " << loaded << " previously registered libraries" << std::endl;
  }
}


MainWindow::~MainWindow()
{
}


void
MainWindow::addLibrary(const char* label, const char* path)
{
  Library *newlib = new Library(label, path);
  libraries.push_back(newlib);
}


void
MainWindow::addLibraryButton(const char* label, const char* /*path*/)
{
  /* when we have more than this many buttons, we go smaller */
  const size_t LAYOUT_SHIFT = 20;
  const size_t COLUMNS = 5;
  static const QSize SMALLER_SIZE = QSize(128, 64);
  static const QSize DEFAULT_SIZE = QSize(128, 128);

  /* count how many buttons we got */
  size_t buttons = 0;
  QLayoutItem* item;
  for (size_t i = 0; i < (size_t)ui.gridLayout->count(); ++i) {
    item = ui.gridLayout->itemAt(i);
    if (!item) {
      break;
    }

    QPushButton* button = qobject_cast<QPushButton*>(item->widget());
    if (button)
      buttons++;
  }

  /* start making a new button */
  QPushButton *newButton = new QPushButton(label, this);
  if (buttons >= LAYOUT_SHIFT) {
    newButton->setFixedSize(SMALLER_SIZE);
  } else {
    newButton->setFixedSize(DEFAULT_SIZE);
  }
  QFont font = newButton->font();
  font.setPointSize(20);
  newButton->setFont(font);

  // TODO: adjust button properties to connect signal to slot
  //    ui.gridLayout->addWidget(newButton); // TODO: need a ui.layout->addWidget

  /* add our new button */
  if (buttons % COLUMNS == 0) {
    ui.gridLayout->addWidget(newButton, buttons / COLUMNS, buttons % COLUMNS);
  } else {
    size_t rows = ui.gridLayout->rowCount();
    ui.gridLayout->addWidget(newButton, rows-1, buttons % COLUMNS);
  }

  /* once we have a lot of buttons, make them all smaller */
  if (buttons == LAYOUT_SHIFT) {
    item = ui.gridLayout->itemAt(0);
    QPushButton* addButton = qobject_cast<QPushButton*>(item->widget());
    if (addButton) {
      QFont buttonFont = addButton->font();
      buttonFont.setPointSize(50);
      addButton->setFont(buttonFont);
    }

    for (size_t i = 0; i < (size_t)ui.gridLayout->count(); ++i) {
      item = ui.gridLayout->itemAt(i);
      if (item && item->widget()) {
        QPushButton* button = qobject_cast<QPushButton*>(item->widget());
        if (button) {
          button->setFixedSize(SMALLER_SIZE);
        }
      }
    }
  }
}


void
MainWindow::updateStatusLabel(const char* status)
{
  ui.indexingStatus->setText(status);
}


void
MainWindow::on_addLibraryButton_clicked()
{
  QString folderPath = QFileDialog::getExistingDirectory(this, tr("Select Folder"), ".");
  if (!folderPath.isEmpty()) {
    // Create a new button for the selected folder
    QString name = QString(folderPath.toStdString().substr(folderPath.toStdString().find_last_of("/\\") + 1).c_str());

    /* when we click +, add path to our libraries and add a button for it */
    addLibrary(name.toStdString().c_str(), folderPath.toStdString().c_str());
    addLibraryButton(name.toStdString().c_str(), folderPath.toStdString().c_str());
    saveState();
  }
}


size_t
MainWindow::saveState()
{
  QSettings settings("BRL-CAD", "CADventory");
  settings.beginWriteArray("libraries");
  size_t index = 0;
  for(auto lib : libraries) {
    settings.setArrayIndex(index++);
    settings.setValue("name", lib->name());
    settings.setValue("path", lib->path());
  }
  settings.endArray();

  return index;
}


size_t
MainWindow::loadState() {
  QSettings settings("BRL-CAD", "CADventory");
  size_t size = settings.beginReadArray("libraries");
  for (size_t i = 1; i < size; ++i) {
    settings.setArrayIndex(i);
    addLibrary(settings.value("name").toString().toStdString().c_str(), settings.value("path").toString().toStdString().c_str());
    addLibraryButton(settings.value("name").toString().toStdString().c_str(), settings.value("path").toString().toStdString().c_str());
  }
  settings.endArray();

  return size;
}
