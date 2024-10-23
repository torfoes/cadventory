#include "./LibraryWindow.h"
#include "./Model.h"
#include "./ProcessGFiles.h"
#include "config.h"

#include <QStringListModel>
#include <QListWidgetItem>
#include <QStringList>
#include <QString>
#include <QFileInfo>
#include <QPdfWriter>
#include <QPainter>
#include <QPixmap>
#include <QStyledItemDelegate>
#include <QFileDialog>
#include <QDir>
#include <QStandardPaths>

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

LibraryWindow::LibraryWindow(QWidget* parent) : QWidget(parent)
{
    this->setFixedSize(QSize(876, 600));
    ui.setupUi(this);

    /* make Model selections update the tabs */
    connect(ui.listWidget, &QListWidget::currentItemChanged, this, &LibraryWindow::onModelSelectionChanged);
    connect(ui.generateReport, &QPushButton::pressed, this, &LibraryWindow::generateReport);
}

LibraryWindow::~LibraryWindow()
{
}

void LibraryWindow::generateReport()
{
    // Open a file dialog for the user to choose the save location
    QString filepath = QFileDialog::getSaveFileName(
        this,
        tr("Save Report"),
        QDir::homePath(), // Default directory
        tr("PDF Files (*.pdf)")
        );

    if (filepath.isEmpty())
        return; // User canceled the save dialog

    QPdfWriter pdfWriter(filepath);

    // Set the resolution (optional)
    pdfWriter.setResolution(300);

    // Set the page size (A4, Letter, etc.)
    pdfWriter.setPageSize(QPageSize(QPageSize::A4));

    // Create a QPainter to draw on the QPdfWriter
    QPainter painter(&pdfWriter);

    // Set a font for the text
    QFont font("Helvetica", 18);
    painter.setFont(font);
    painter.drawText(750, 200, "Cadventory");
    painter.setPen(QPen(Qt::black, 3));
    painter.drawLine(-200, 250, 2450, 250);
    painter.drawLine(-200, 3400, 2450, 3400);

    QFont font_two("Helvetica", 6);
    painter.setFont(font_two);
    painter.drawText(300, 300, QString("Library: %1").arg(library->name()));

    int x = 325;
    int y = 350;
    painter.drawText(x, y, "Geometry");
    y += 25;

    for (const auto& str : geometryModel->stringList()) {
        y += 25;
        painter.drawText(x, y, str);
    }
    y += 50;
    painter.drawText(x, y, "Images");
    y += 25;
    for (const auto& str : imagesModel->stringList()) {
        y += 25;
        painter.drawText(x, y, str);
    }
    y += 50;
    painter.drawText(x, y, "Documents");
    y += 25;
    for (const auto& str : documentsModel->stringList()) {
        y += 25;
        painter.drawText(x, y, str);
    }

    ProcessGFiles gFileProcessor; // To call commands
    int num_file = 0;

    // Use the system's temporary directory
    QString temp_dir = QDir::tempPath() + "/cadventory/";
    QDir tempDir(temp_dir);
    if (!tempDir.exists()) {
        tempDir.mkpath(".");
    }

    std::string temp_dir_str = temp_dir.toStdString();

    std::cout << "Generating gist reports" << std::endl;

    for (auto str : report) {
        std::cout << "Model selected: " << str << std::endl;
        std::string model_to_gist = str;
        std::string path_gist_output = std::to_string(num_file) + ".png";
        std::string gist_command = std::string(GIST_EXECUTABLE_PATH) + " " + model_to_gist + " -o " + temp_dir_str + path_gist_output;

        auto [output, error] = gFileProcessor.runCommand(gist_command);
        std::cout << "Std output: " << output << std::endl;
        std::cout << "Std error: " << error << std::endl;
        num_file++;
    }

    painter.rotate(90);
    for (int k = 0; k < num_file; k++) {
        std::string png = temp_dir_str + std::to_string(k) + ".png";
        QString png_qstr = QString::fromStdString(png);
        QPixmap gist(png_qstr);
        pdfWriter.newPage();
        painter.drawPixmap(0, -2408, gist);
    }
    painter.end();
    std::cout << "Report Generated" << std::endl;
}

void LibraryWindow::loadFromLibrary(Library* _library)
{
    this->library = _library;
    this->setWindowTitle(library->name() + QString(" Library"));
    this->ui.currentLibrary->setText(library->name());

    /* start fresh */
    ui.listWidget->clear();

    /* populate Models listing */
    auto modelDirs = library->getModels();
    for (const auto& dir : modelDirs) {
        QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(dir));
        ui.listWidget->addItem(item);
    }

    /* prepare MVC models for our list views */
    auto populateModel = [this](QStringListModel* listModel, const std::vector<std::string>& fullPaths) {
        QStringList list;
        std::string base = std::string(library->path()) + "/";
        for (const auto& fullPath : fullPaths) {
            std::string relativePath = fullPath.substr(base.length());
            list << QString::fromStdString(relativePath);
        }
        listModel->setStringList(list);
    };

    geometryModel = new QStringListModel(this);
    imagesModel = new QStringListModel(this);
    documentsModel = new QStringListModel(this);
    dataModel = new QStringListModel(this);
    tagsModel = new QStringListModel(this);
    currentTagsModel = new QStringListModel(this);
    currentPropertiesModel = new QStringListModel(this);

    /* populate the MVC models */
    populateModel(geometryModel, library->getGeometry());
    populateModel(imagesModel, library->getImages());
    populateModel(documentsModel, library->getDocuments());
    populateModel(dataModel, library->getData());

    /* wire the MVC models to the list views */
    ui.geometryListView->setModel(geometryModel);
    ui.imagesListView->setModel(imagesModel);
    ui.documentsListView->setModel(documentsModel);
    ui.dataListView->setModel(dataModel);

    std::cout << "Setting tags model" << std::endl;
    ui.tagsListView->setModel(tagsModel);
    std::cout << "Setting current tags model" << std::endl;
    ui.currentTagsListView->setModel(currentTagsModel);
    std::cout << "Setting current properties model" << std::endl;
    ui.currentPropertiesListView->setModel(currentPropertiesModel);
    std::cout << "Set current properties model" << std::endl;

    /* load tags */
    std::cout << "Calling display tags for library: " << library->name() << std::endl;
    loadTags();
    std::cout << "Called display tags for library: " << library->name() << std::endl;

    QString root_folder = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/cadventory/previews/";
    QDir rootDir(root_folder);
    if (!rootDir.exists()) {
        rootDir.mkpath(".");
    }

    for (const QString& path : geometryModel->stringList()) {
        // Check if the string ends with ".g"
        if (path.endsWith(".g", Qt::CaseInsensitive)) {
            QString filename = QFileInfo(path).fileName();
            std::cout << filename.toStdString() << std::endl;

            QString filepath = root_folder + filename;
            filepath.chop(2); // Remove last 2 characters (".g")
            filepath.append(".png");
            std::cout << filepath.toStdString() << std::endl;

            QListWidgetItem* item = new QListWidgetItem(QIcon(filepath), filename);
            QList<QSize> availableSizes = item->icon().availableSizes();

            if (availableSizes.isEmpty()) {
                delete item;
            } else {
                ui.listWidgetPage->addItem(item);
            }
        }
    }

    ui.listWidgetPage->setResizeMode(QListView::Adjust);
    ui.listWidgetPage->setViewMode(QListView::IconMode);

    ui.listWidgetPage->setWordWrap(true);

    ui.listWidgetPage->setIconSize(QSize(72, 72));
    ui.listWidgetPage->setGridSize(QSize(144, 108));
    ui.listWidgetPage->setUniformItemSizes(true);
    ui.listWidgetPage->setMovement(QListView::Static);
    ui.listWidgetPage->setResizeMode(QListView::Adjust);
    ui.listWidgetPage->setLayoutMode(QListView::Batched);
    ui.listWidgetPage->setBatchSize(10);

    QStyledItemDelegate* delegate = new QStyledItemDelegate(this);
    ui.listWidgetPage->setItemDelegate(delegate);
}

void LibraryWindow::on_allLibraries_clicked()
{
    this->hide();
    main->show();
}

void LibraryWindow::updateListModelForDirectory(QStringListModel* listModel, const std::vector<std::string>& allItems, const std::string& directory)
{
    QStringList filteredItems;
    std::string base = library->path() + (directory == "." ? "" : "/" + directory);
    for (const auto& item : allItems) {
        if (item.find(base) == 0) { // item starts with base path
            std::string relativePath = item.substr(base.length() + 1); // +1 to skip leading slash
            filteredItems << QString::fromStdString(relativePath);
        }
    }
    listModel->setStringList(filteredItems);
}

void LibraryWindow::onModelSelectionChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
    if (!current)
        return; // Nothing selected

    QString selectedDir = current->text();
    int modelId = library->model->hashModel(library->fullPath + "/" + selectedDir.toStdString());

    std::vector<std::string> modelTags = library->model->getTagsForModel(modelId);
    std::cout << ">>Selected model: " << selectedDir.toStdString() << std::endl;

    QStringList qModelTags;
    for (const auto& tag : modelTags) {
        qModelTags << QString::fromStdString(tag);
    }
    currentTagsModel->setStringList(qModelTags);

    QStringList qModelProperties;
    std::map<std::string, std::string> modelProperties = library->model->getProperties(modelId);
    for (const auto& [key, value] : modelProperties) {
        qModelProperties << QString::fromStdString(key + ": " + value);
    }
    currentPropertiesModel->setStringList(qModelProperties);
}

void LibraryWindow::loadTags()
{
    std::cout << "Displaying tags for library: " << library->name() << std::endl;

    std::vector<std::string> sortedTags = library->getTags();
    for (const auto& tag : sortedTags) {
        std::cout << tag << " ";
    }
    std::cout << std::endl;
    QStringList tags;
    for (const auto& tag : sortedTags) {
        tags << QString::fromStdString(tag);
    }
    std::cout << "Setting tags model" << std::endl;
    tagsModel->setStringList(tags);
}

void displayModel(const ModelData& model)
{
    std::cout << ">>>Model: " << model.short_name << std::endl;
    std::cout << "Primary file: " << model.primary_file << std::endl;
    std::cout << "Override info: " << model.override_info << std::endl;
    std::cout << "Properties: " << std::endl;
    for (const auto& [key, value] : model.properties) {
        std::cout << "  " << key << ": " << value << std::endl;
    }
}

void LibraryWindow::on_pushButton_clicked()
{
    report = this->library->getModels();
    for (std::string& path : report) {
        path = library->fullPath + "/" + path;
    }
}

void LibraryWindow::on_listWidgetPage_itemClicked(QListWidgetItem* item)
{
    std::string name = item->text().toStdString();

    int i = 0;
    for (const auto& file : report) {
        std::filesystem::path filePath(file);
        std::string filename = filePath.filename().string();

        if (name == filename) {
            report.erase(report.begin() + i);
            std::cout << filename + " is unselected from report" << std::endl;
            return;
        }
        i++;
    }
    report.push_back(this->library->fullPath + "/" + name);
    std::cout << this->library->fullPath + "/" + name << std::endl;
    std::cout << name + " is selected for report" << std::endl;
}
