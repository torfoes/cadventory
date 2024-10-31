#ifndef LIBRARYWINDOW_H
#define LIBRARYWINDOW_H

#include <QWidget>
#include <QListView>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QThread>

#include "ui_librarywindow.h"
#include "Library.h"
#include "Model.h"
#include "ModelCardDelegate.h"
#include "ModelFilterProxyModel.h"
#include "IndexingWorker.h"

class MainWindow;

class LibraryWindow : public QWidget {
    Q_OBJECT

public:
    explicit LibraryWindow(QWidget* parent = nullptr);
    ~LibraryWindow();

    void loadFromLibrary(Library* _library);
    void setMainWindow(MainWindow* mainWindow);


private slots:
    void onSearchTextChanged(const QString& text);
    void onSearchFieldChanged(const QString& field);
    void onAvailableModelClicked(const QModelIndex& index);
    void onSelectedModelClicked(const QModelIndex& index);
    void onGenerateReportButtonClicked();
    void on_backButton_clicked();

    void onSettingsClicked(int modelId);

    void onGeometryBrowserClicked(int modelId);
    void onModelViewClicked(int modelId);

    void startIndexing();
    void onModelProcessed(int modelId);

private:
    void setupModelsAndViews();
    void setupConnections();

    Ui::LibraryWindow ui;
    Library* library;
    MainWindow* mainWindow;

    Model* model;

    // Proxy models
    ModelFilterProxyModel* availableModelsProxyModel;
    ModelFilterProxyModel* selectedModelsProxyModel;

    // Delegates
    ModelCardDelegate* modelCardDelegate;

    // Selected models list
    QList<int> selectedModelIds;

    // Indexing worker and thread
    QThread* indexingThread;
    IndexingWorker* indexingWorker;
};

#endif // LIBRARYWINDOW_H
