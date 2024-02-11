#include "CADventory.h"

#include <QPixmap>
#include <QSplashScreen>

#include <iostream>
#include <unistd.h>
#include <filesystem>

#ifdef _WIN32
int APIENTRY
WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpszCmdLine,
	int nCmdShow)
{
    int argc = __argc;
    char **argv = __argv;
#else
int
main(int argc, char **argv)
{
#endif
  CADventory app(argc, argv);
  //  QApplication app(argc, argv);

  // std::cout << std::filesystem::current_path()<< std::endl;
  QPixmap pixmap("../splash.png");
  if (pixmap.isNull()) {
    pixmap = QPixmap(512, 512);
    pixmap.fill(Qt::white);
  }
  QSplashScreen splash(pixmap);
  splash.show();
  splash.showMessage("Loading from main... please wait.");
  //  app.processEvents(QEventLoop::AllEvents);
  app.processEvents();

  sleep(1);

  return app.exec();
}
