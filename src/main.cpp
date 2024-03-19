#include "CADventory.h"

#include <QTimer>
#include <QPixmap>
#include <QSplashScreen>

#include <stdlib.h>
#include <iostream>
#include <filesystem>


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

int APIENTRY
WinMain(HINSTANCE /*hInstance*/,
	HINSTANCE /*hPrevInstance*/,
	LPSTR /*lpszCmdLine*/,
	int /*nCmdShow*/)
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
    const char *home = getenv("HOME");
    if (!home)
      home = "~";
    app.processEvents();
    std::string localDir = std::string(home) + std::string("/.CADventory");
    app.indexDirectory(localDir.c_str());
  });

  return app.exec();
}
