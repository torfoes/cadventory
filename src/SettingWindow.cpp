#include "SettingWindow.h"
#include "ui_SettingWindow.h"


SettingWindow::SettingWindow(MainWindow *mainwindow,QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingWindow)
{
    ui->setupUi(this);
   main = mainwindow;


}

SettingWindow::~SettingWindow()
{
    delete ui;
}

void SettingWindow::on_enablePreview_stateChanged(int state)
{
    if(state == Qt::Checked){
        main->setPreviewFlag(true);
    }
    else if(state == Qt::Unchecked)
    {
        main->setPreviewFlag(false);
    }

}

