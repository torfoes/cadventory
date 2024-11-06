// MainWindow.cpp

#include "SettingWindow.h"
#include "MainWindow.h"
#include "LibraryWindow.h"

#include <iostream>
#include <QFileDialog>
#include <QPushButton>
#include <QSettings>
#include <QMessageBox>
#include <QMenuBar>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    // Clear settings (for development/testing purposes)
    // QSettings settings;
    // settings.clear();


    this->setFixedSize(QSize(876, 600));
    ui.setupUi(this);

    // Adjust the + button label position
    QLayoutItem* item = ui.gridLayout->itemAt(0);
    QPushButton* addButton = qobject_cast<QPushButton*>(item->widget());
    if (addButton) {
        addButton->setStyleSheet("QPushButton { padding-top: -10px; }");
    }

    // Explain how to reset the list
    std::cout << "To manually reset on Mac:" << std::endl
              << "  defaults delete org.brlcad.CADventory" << std::endl;
    std::cout << "To reset via app, run with --no-gui option." << std::endl;

    // Load previously saved libraries
    size_t loaded = loadState();
    if (loaded) {
        std::cout << "Loaded " << loaded << " previously registered libraries" << std::endl;
    }

    QString home = QDir::homePath();
    addLibrary("Local Home", home.toStdString().c_str());
    std::cout << "Loaded local home [" << home.toStdString().c_str() << "] library" << std::endl;

    fileMenu = new QMenu(tr("&File"),this);
    editMenu = new QMenu(tr("&Edit"),this);
    viewMenu = new QMenu(tr("&View"),this);
    windowMenu = new QMenu(tr("&Window"),this);
    helpMenu = new QMenu(tr("&Help"),this);

    ui.librarywidget->hide();



    QAction *set = new QAction(tr("&General Settings"),this);

    windowMenu->addAction(set);

    QAction *test = new QAction(tr("&test"),this);

    fileMenu->addAction(test);
    windowMenu->addAction(test);
    viewMenu->addAction(test);
    helpMenu->addAction(test);


    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(editMenu);
    menuBar()->addMenu(viewMenu);
    menuBar()->addMenu(windowMenu);
    menuBar()->addMenu(helpMenu);

    settingWindow = new SettingWindow(this);
    connect(set,&QAction::triggered,this,&MainWindow::showSettingsWindow);

}

MainWindow::~MainWindow()
{
    // Clean up dynamically allocated libraries
    for (Library* lib : libraries) {
        delete lib;
    }
}

void MainWindow::addLibrary(const char* label, const char* path)
{
    for (Library* lib : libraries) {
        if (QString(lib->name()) == QString(label) && QString(lib->path()) == QString(path)) {
            std::cout << "Library [" << label << "] already exists, skipping add." << std::endl;
            return;
        }
    }

    std::cout << "Adding library [" << label << "] => " << path << std::endl;

    Library* newlib = new Library(label, path);
    libraries.push_back(newlib);
    size_t files = newlib->indexFiles();

    QString libCount = QString("Scanned ") + QString::number(files) + QString(" file(s) in ") + label;
    this->updateStatusLabel(libCount.toStdString().c_str());

    // Add a button for the new library
    addLibraryButton(label, path);
}


void MainWindow::openLibrary()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button)
        return;

    QString lookupKey = button->text();
    Library* foundLibrary = nullptr;

    for (Library* lib : libraries) {
        if (QString(lib->name()) == lookupKey) {
            foundLibrary = lib;
            break;
        }
    }

    if (foundLibrary) {
        LibraryWindow* libraryWindow = new LibraryWindow(this->centralWidget());
        std::cout << "Opening library " << foundLibrary->name() << std::endl;

        // Set the main window pointer using a setter method
        libraryWindow->setMainWindow(this);

        // Load the library into the window (this should now start the indexing in the background)
        libraryWindow->loadFromLibrary(foundLibrary);

        std::cout << "Loaded library " << foundLibrary->name() << std::endl;

        // Hide the main window and show the library window
        //this->hide();
        //libraryWindow->show();
        ui.librarywidget=libraryWindow;

        ui.origin->hide();
        setWindowTitle(foundLibrary->name() + QString(" Library"));
        ui.librarywidget->setAttribute(Qt::WA_DeleteOnClose);
        ui.librarywidget->show();

    } else {
        QMessageBox::warning(this, "Library Not Found", "Could not find the library for " + lookupKey);
    }
}

void MainWindow::addLibraryButton(const char* label, const char* /*path*/)
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

    connect(newButton, &QPushButton::released, this, &MainWindow::openLibrary);

    /* add our new button */
    int row = (buttons) / COLUMNS;
    int column = (buttons) % COLUMNS;
    ui.gridLayout->addWidget(newButton, row, column);

    /* once we have a lot of buttons, make them all smaller */
    if (buttons == LAYOUT_SHIFT) {
        for (int i = 0; i < ui.gridLayout->count(); ++i) {
            QLayoutItem* item = ui.gridLayout->itemAt(i);
            if (item && item->widget()) {
                QPushButton* button = qobject_cast<QPushButton*>(item->widget());
                if (button) {
                    button->setFixedSize(SMALLER_SIZE);
                }
            }
        }
    }
}

void MainWindow::updateStatusLabel(const char* status)
{
    ui.indexingStatus->setText(status);
}

void MainWindow::on_addLibraryButton_clicked()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, tr("Select Folder"), ".");
    if (!folderPath.isEmpty()) {
        QString name = QFileInfo(folderPath).fileName();

        // Add the library and its button
        addLibrary(name.toStdString().c_str(), folderPath.toStdString().c_str());
        saveState();
    }
}

size_t MainWindow::saveState()
{
    QSettings settings;
    settings.beginWriteArray("libraries");
    size_t index = 0;
    for (auto lib : libraries) {
        if (QString(lib->name()) == QString("Local Home"))
            continue;
        settings.setArrayIndex(index++);
        settings.setValue("name", lib->name());
        settings.setValue("path", lib->path());
    }
    settings.endArray();

    return index;
}

size_t MainWindow::loadState()
{
    QSettings settings;
    size_t size = settings.beginReadArray("libraries");
    for (size_t i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString name = settings.value("name").toString();
        QString path = settings.value("path").toString();

        if (name == "Local Home") {
            continue;
        }
        addLibrary(name.toStdString().c_str(), path.toStdString().c_str());
    }
    settings.endArray();

    return size;
}

void MainWindow::on_homeLibraryButton_clicked()
{
    QPushButton* button = ui.homeLibraryButton;
    if (button) {
        openLibrary();
    }
}

void MainWindow::showSettingsWindow()
{
    settingWindow->show();
}

void MainWindow::setPreviewFlag(bool state)
{
    previewFlag = state;
}

void MainWindow::returnCentralWidget()
{

    ui.librarywidget->hide();
    setWindowTitle( QString("Main Window"));
    ui.origin->show();

}
