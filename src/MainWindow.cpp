
#include "./MainWindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QThreadPool>
#include <QtConcurrent/QtConcurrentRun>
#include <iostream>

#include "LibraryWindow.h"

// LibraryWindow* libraryWindow;

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
  this->setFixedSize(QSize(876, 600));
  ui.setupUi(this);

  /* adjust the + button label position */
  QLayoutItem* item = ui.gridLayout->itemAt(0);
  QPushButton* addButton = qobject_cast<QPushButton*>(item->widget());
  if (addButton) {
    addButton->setStyleSheet("QPushButton { padding-top: -10px; }");
  }

  /* explain how to reset the list */
  std::cout << "To manually reset on Mac:" << std::endl
            << "  defaults delete org.brlcad.CADventory" << std::endl;
  std::cout << "To reset via app, run with --no-gui option." << std::endl;

  size_t loaded = loadState();
  if (loaded) {
    std::cout << "Loaded " << loaded << " previously registered libraries"
              << std::endl;
  }

  QString home = QDir::homePath();
  addLibrary("Local Home", home.toStdString().c_str());
  std::cout << "Loaded local home [" << home.toStdString().c_str()
            << "] library" << std::endl;

  /* connect the default home dir button */
  connect(ui.homeLibraryButton, &QPushButton::released, this,
          &MainWindow::openLibrary);
}

MainWindow::~MainWindow() {}

void MainWindow::addLibrary(const char* label, const char* path) {
  std::cout << "Adding library [" << label << "] => " << path << std::endl;

  Library* newlib = new Library(label, path);
  libraries.push_back(newlib);
  size_t files = newlib->indexFiles();

  QString libCount = QString("Scanned ") + QString::number(files) +
                     QString(" file(s) in ") + label;
  this->updateStatusLabel(libCount.toStdString().c_str());
}

void MainWindow::openLibrary() {
  QPushButton* button = qobject_cast<QPushButton*>(sender());
  if (!button) return;

  QString lookupKey = button->text();
  Library* foundLibrary = nullptr;

  for (Library* lib : libraries) {
    // std::cout << "Looking for " << lib->name() << " == " <<
    // lookupKey.toStdString() << std::endl;

    if (lib->name() == lookupKey) {
      foundLibrary = lib;
      break;
    }
  }

  if (foundLibrary) {
    LibraryWindow* libraryWindow = new LibraryWindow(nullptr);
    std::cout << "Opening library " << foundLibrary->name() << std::endl;
    // foundLibrary->loadDatabase();
    libraryWindow->loadFromLibrary(foundLibrary);
    std::cout << "Loaded library " << foundLibrary->name() << std::endl;

    libraryWindow->show();
  } else {
    QMessageBox::warning(this, "Library Not Found",
                         "Could not find the library for " + lookupKey);
  }
}

void MainWindow::addLibraryButton(const char* label, const char* /*path*/) {
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
    if (button) buttons++;
  }

  /* start making a new button */
  QPushButton* newButton = new QPushButton(label, this);
  if (buttons >= LAYOUT_SHIFT) {
    newButton->setFixedSize(SMALLER_SIZE);
  } else {
    newButton->setFixedSize(DEFAULT_SIZE);
  }
  QFont font = newButton->font();
  font.setPointSize(20);
  newButton->setFont(font);

  connect(newButton, &QPushButton::released, this, &MainWindow::openLibrary);

  /* add our new button */
  if (buttons % COLUMNS == 0) {
    ui.gridLayout->addWidget(newButton, buttons / COLUMNS, buttons % COLUMNS);
  } else {
    size_t rows = ui.gridLayout->rowCount();
    ui.gridLayout->addWidget(newButton, rows - 1, buttons % COLUMNS);
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

void MainWindow::updateStatusLabel(const char* status) {
  ui.indexingStatus->setText(status);
}

void MainWindow::on_addLibraryButton_clicked() {
  QString folderPath =
      QFileDialog::getExistingDirectory(this, tr("Select Folder"), ".");
  // libraries.clear();
  if (!folderPath.isEmpty()) {
    // Create a new button for the selected folder
    QString name =
        QString(folderPath.toStdString()
                    .substr(folderPath.toStdString().find_last_of("/\\") + 1)
                    .c_str());

    // When we click +, add path to our libraries and add a button for it
    Library* newLibrary = new Library(name.toStdString().c_str(),
                                      folderPath.toStdString().c_str());
    libraries.push_back(newLibrary);
    addLibraryButton(name.toStdString().c_str(),
                     folderPath.toStdString().c_str());

    // Save the state of the application
    saveState();

    // Immediately load the database and process the .g files
    newLibrary->createDatabase(this);

    // Optionally, update the UI status to indicate processing completion
    updateStatusLabel("Library added and .g files processed.");
  }
}

size_t MainWindow::saveState() {
  QSettings settings;
  settings.beginWriteArray("libraries");
  size_t index = 0;
  for (auto lib : libraries) {
    if (QString(lib->name()) == QString("Local Home")) continue;
    settings.setArrayIndex(index++);
    settings.setValue("name", lib->name());
    settings.setValue("path", lib->path());
  }
  settings.endArray();

  return index;
}

size_t MainWindow::loadState() {
  QSettings settings;
  size_t size = settings.beginReadArray("libraries");
  for (size_t i = 0; i < size; ++i) {
    settings.setArrayIndex(i);
    QString name = settings.value("name").toString();
    QString path = settings.value("path").toString();
    addLibrary(name.toStdString().c_str(), path.toStdString().c_str());
    addLibraryButton(name.toStdString().c_str(), path.toStdString().c_str());
  }
  settings.endArray();

  return size;
}

void MainWindow::on_homeLibraryButton_clicked() {
  // hide();
  // libraryWindow->show();
}
