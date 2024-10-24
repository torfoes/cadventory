// LibraryWindow.h

#ifndef LIBRARYWINDOW_H
#define LIBRARYWINDOW_H

#include <QWidget>
#include <QListView>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QSortFilterProxyModel>

#include <QThread>
#include "IndexingWorker.h"

#include "ui_LibraryWindow.h"
#include "Library.h"
#include "Model.h"
#include "ModelCardDelegate.h"
#include "ModelFilterProxyModel.h"

class MainWindow;

class LibraryWindow : public QWidget {
    Q_OBJECT

public:
    explicit LibraryWindow(QWidget* parent = nullptr);
    ~LibraryWindow();

    void loadFromLibrary(Library* _library);
    void setMainWindow(MainWindow* mainWindow);

private slots:
    // Slot for when the search text changes
    void onSearchTextChanged(const QString& text);
    // Slot for when the search field changes
    void onSearchFieldChanged(const QString& field);
    // Slot for when a model card is clicked in the available models view
    void onAvailableModelClicked(const QModelIndex& index);
    // Slot for when a model card is clicked in the selected models view
    void onSelectedModelClicked(const QModelIndex& index);
    // Slot for when the Generate Report button is clicked
    void onGenerateReportButtonClicked();
    void onBackButtonClicked();

    // Slot for when settings are clicked in the model card
    void onSettingsClicked(int modelId);

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
