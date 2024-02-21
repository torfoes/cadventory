#include "CADventory.h"

#include <QTimer>
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
  app.showSplash();

  QTimer::singleShot(250, [&app]() {
    app.processEvents();
    app.indexDirectory("/Users/morrison");
  });

  return app.exec();
}
