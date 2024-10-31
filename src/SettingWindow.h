#ifndef SETTINGWINDOW_H
#define SETTINGWINDOW_H

#include <QDialog>
#include "MainWindow.h"

class MainWindow;

namespace Ui {
class SettingWindow;
}

class SettingWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SettingWindow(MainWindow *mainwindow,QWidget *parent = nullptr);
    ~SettingWindow();
    MainWindow* main;

private slots:
    void on_enablePreview_stateChanged(int state);

private:
    Ui::SettingWindow *ui;
};

#endif // SETTINGWINDOW_H
