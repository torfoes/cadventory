#ifndef CADVENTORY_H
#define CADVENTORY_H

#include <QApplication>
#include <QMainWindow>
#include <QSplashScreen>
#include <QObject>


class CADventory : public QApplication
{
  Q_OBJECT

public:
  CADventory(int &argc, char *argv[]);
  ~CADventory();
  void showSplash();
  void indexDirectory(const char *path);

private:
  void initMainWindow();

public:
  QMainWindow *window;
  QSplashScreen *splash;
};

#endif /* CADVENTORY_H */
