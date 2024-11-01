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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void addLibrary(const char* label = nullptr, const char* path = nullptr);
    void openLibrary();
    const std::vector<Library*>& getLibraries() const { return libraries; }
    size_t publicSaveState() { return saveState(); }
    size_t publicLoadState() { return loadState(); }
    void clearLibraries();

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
