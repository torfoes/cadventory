#ifndef LIBRARYWINDOW_H
#define LIBRARYWINDOW_H

#include <map>
#include <string>
#include <vector>

#include <QWidget>
#include <QObject>

#include "./ui_librarywindow.h"

#include "./Library.h"


class LibraryWindow : public QWidget
{
  Q_OBJECT

public:
  explicit LibraryWindow(QWidget* parent = nullptr);
  ~LibraryWindow();

  void loadFromLibrary(Library *library);

private:
  Ui::LibraryWindow ui;
  Library* library;
};

#endif /* LIBRARYWINDOW_H */
