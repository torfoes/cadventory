// MainWindow.h

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <map>
#include <string>
#include <vector>

#include <QMainWindow>
#include <QObject>

#include "ui_mainwindow.h"

#include "Library.h"
#include "SettingWindow.h"

class SettingWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void addLibrary(const char* label = nullptr, const char* path = nullptr);
    void openLibrary();
    void showSettingsWindow();
    void setPreviewFlag(bool state);
    SettingWindow *setting;
    bool previewFlag = true;
    QMenu * fileMenu;
    QMenu * editMenu;
    QMenu * viewMenu;
    QMenu * windowMenu;
    QMenu * helpMenu;


public slots:
    void updateStatusLabel(const char* status);

private slots:
    void on_addLibraryButton_clicked();
    void on_homeLibraryButton_clicked();

protected:
    size_t saveState();
    size_t loadState();

    void addLibraryButton(const char* label = nullptr, const char* path = nullptr);

private:
    Ui::MainWindow ui;
    std::vector<Library*> libraries;

};

#endif /* MAINWINDOW_H */
