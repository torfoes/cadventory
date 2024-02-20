#include "CADventory.h"

#include <QMainWindow>
#include <QPixmap>
#include <QSplashScreen>

#include <unistd.h>


CADventory::CADventory(int &argc, char *argv[]) : QApplication (argc, argv)
{
  setOrganizationName("BRL-CAD");
  setOrganizationDomain("brlcad.org");
  setApplicationName("CADventory");
  setApplicationVersion("0.1.0");

  //  w = new MainWindow();
  QPixmap pixmap("/Users/morrison/Desktop/RSEG127/cadventory/splash.png");
  if (pixmap.isNull()) {
    pixmap = QPixmap(512, 512);
    pixmap.fill(Qt::white);
  }
  QSplashScreen splash(pixmap);
  splash.show();
  splash.showMessage("Loading... please wait.");
  this->processEvents();

  sleep(1);

  QMainWindow window;
  window.show();
  this->processEvents();

  //  splash.finish(&splash);
}


CADventory::~CADventory()
{
}
