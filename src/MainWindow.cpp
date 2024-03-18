
#include "MainWindow.h"

#include <iostream>
#include <QFileDialog>
#include <QPushButton>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
  this->setFixedSize(QSize(876, 600));
  ui.setupUi(this);
}


MainWindow::~MainWindow()
{
}


void
MainWindow::updateStatusLabel(const char *status)
{
  ui.indexingStatus->setText(status);
}


void
MainWindow::on_addLibraryButton_clicked()
{
  QString folderPath = QFileDialog::getExistingDirectory(this, tr("Select Folder"), "/path/to/default/directory");
  if (!folderPath.isEmpty()) {
    // Create a new button for the selected folder
    QString name = QString(folderPath.toStdString().substr(folderPath.toStdString().find_last_of("/\\") + 1).c_str());

    QPushButton *newButton = new QPushButton(name, this);
    newButton->setFixedSize(QSize(128, 128));
    QFont font = newButton->font();
    font.setPointSize(20);
    newButton->setFont(font);

    size_t buttons = ui.gridLayout->count();
    std::cout << "count is " << buttons << std::endl;
    // TODO: adjust button properties to connect signal to slot
    //    ui.gridLayout->addWidget(newButton); // TODO: need a ui.layout->addWidget
    if (buttons % 4 == 0) {
      ui.gridLayout->addWidget(newButton, buttons / 4, buttons % 4);
    } else {
      size_t rows = ui.gridLayout->rowCount();
      ui.gridLayout->addWidget(newButton, rows-1, buttons % 4);
    }
  }
}
