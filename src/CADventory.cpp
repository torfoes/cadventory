#include "CADventory.h"

#include <QPixmap>
#include <QTimer>

#include "MainWindow.h"


CADventory::CADventory(int &argc, char *argv[]) : QApplication (argc, argv), window(nullptr), splash(nullptr)
{
  setOrganizationName("BRL-CAD");
  setOrganizationDomain("brlcad.org");
  setApplicationName("CADventory");
  setApplicationVersion("0.1.0");

  QString appName = QCoreApplication::applicationName();
  QString appVersion = QCoreApplication::applicationVersion();

  // ANSI escape codes for underlining and reset
  QString underlineStart = "\033[4m";
  QString underlineEnd = "\033[0m";

  // print underlined application name and version
  qInfo().noquote() << underlineStart + appName + " " + appVersion + underlineEnd;
  qInfo() << "Loading ... please wait.";
}


CADventory::~CADventory()
{
  delete window;
  delete splash;
}


void CADventory::initMainWindow()
{
  window = new MainWindow();

  window->show();
  splash->finish(window);
  delete splash;
  splash = nullptr;

  qInfo() << "Done loading.";
}


void CADventory::showSplash()
{
  QPixmap pixmap("/Users/morrison/Desktop/RSEG127/cadventory/splash.png");
  if (pixmap.isNull()) {
    pixmap = QPixmap(512, 512);
    pixmap.fill(Qt::black);
  }
  splash = new QSplashScreen(pixmap);
  splash->showMessage("Loading... please wait.", Qt::AlignLeft, Qt::black);
  splash->show();
  // ensure the splash is displayed immediately
  this->processEvents();

  // keep it visible for a minimum time
  QTimer::singleShot(2000, this, &CADventory::initMainWindow);
}
