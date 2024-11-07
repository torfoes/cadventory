#include "SettingWindow.h"
#include "ui_SettingWindow.h"
#include <QSettings>

SettingWindow::SettingWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingWindow)
{
    ui->setupUi(this);

    // Load settings when the window is initialized
    loadSettings();
}

SettingWindow::~SettingWindow()
{
    delete ui;
}

void SettingWindow::loadSettings()
{
    QSettings settings;
    bool previewFlag = settings.value("previewFlag", true).toBool();
    ui->enablePreview->setChecked(previewFlag);
}

void SettingWindow::saveSettings()
{
    QSettings settings;
    settings.setValue("previewFlag", ui->enablePreview->isChecked());
}

void SettingWindow::on_buttonBox_accepted()
{
    saveSettings();
    accept();
}
