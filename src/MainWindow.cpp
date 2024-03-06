
#include "MainWindow.h"

#include <QFileDialog>
#include <QPushButton>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
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

    // TODO: adjust button properties to connect signal to slot
    ui.gridLayout->addWidget(newButton); // TODO: need a ui.layout->addWidget
  }
}
