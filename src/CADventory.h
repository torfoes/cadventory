#ifndef CADVENTORY_H
#define CADVENTORY_H

#include <QApplication>
#include <QObject>

//#include "MainWindow.h"


class CADventory : public QApplication
{
    Q_OBJECT

    public:
    	CADventory(int &argc, char *argv[]);
	~CADventory();

    public:
  //MainWindow *w = NULL;
};

#endif
