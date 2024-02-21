#include "MainWindow.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
  ui.setupUi(this);
}


MainWindow::~MainWindow()
{
}


void
MainWindow::updateStatusLabel(const char *status) {
  ui.label->setText(status);
}
