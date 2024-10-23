// LibraryWindow.h

#ifndef LIBRARYWINDOW_H
#define LIBRARYWINDOW_H

#include <QWidget>
#include <QListWidgetItem>
#include <vector>
#include <string>

#include "ui_librarywindow.h"
#include "Library.h"
#include "MainWindow.h"

class LibraryWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LibraryWindow(QWidget* parent = nullptr);
    ~LibraryWindow();

    void loadFromLibrary(Library* _library);
    void setMainWindow(MainWindow* mainWindow);

private slots:
    void generateReport();
    void on_allLibraries_clicked();
    void onModelSelectionChanged(QListWidgetItem* current, QListWidgetItem* previous);
    void on_pushButton_clicked();
    void on_listWidgetPage_itemClicked(QListWidgetItem* item);
    void onModelProcessed(int modelId);
    void onIndexingFinished();
    void onGeneratePreviewClicked();

private:
    void setupPreviewsView();
    void startIndexing();

    Ui::LibraryWindow ui;
    Library* library;
    MainWindow* main;

    std::vector<std::string> report;
};

#endif // LIBRARYWINDOW_H
