#include "SettingWindow.h"
#include "ui_SettingWindow.h"
#include <QSettings>


SettingWindow::SettingWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingWindow)
{
    ui->setupUi(this);

    //default
    // if(){
    //     previewFlag = settings.value("preview",);
    // }

}

SettingWindow::~SettingWindow()
{
    delete ui;
}

void SettingWindow::saveSettings()
{

    settings.setValue("preview", previewFlag);
}

void SettingWindow::on_enablePreview_stateChanged(int state)
{
    if(state == Qt::Checked){
        previewFlag=true;
    }
    else if(state == Qt::Unchecked)
    {
        previewFlag=false;
    }

}


void SettingWindow::on_buttonBox_accepted()
{

    saveSettings();
}

