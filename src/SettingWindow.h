#ifndef SETTINGWINDOW_H
#define SETTINGWINDOW_H

#include <QDialog>
#include <QSettings>


namespace Ui {
class SettingWindow;
}

class SettingWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SettingWindow(QWidget *parent = nullptr);
    ~SettingWindow();

    bool previewFlag;

private slots:
    void on_enablePreview_stateChanged(int state);

    void on_buttonBox_accepted();
    void saveSettings();

private:
    Ui::SettingWindow *ui;
    QSettings settings;
};

#endif // SETTINGWINDOW_H
