#include "CADventory.h"

CADventory::CADventory(int &argc, char *argv[]) : QApplication (argc, argv)
{
  setOrganizationName("BRL-CAD");
  setOrganizationDomain("brlcad.org");
  setApplicationName("CADventory");
  setApplicationVersion("0.1.0");

  //  w = new MainWindow();
}


CADventory::~CADventory()
{
}
