
#include "./LibraryWindow.h"
#include "./Model.h"
#include "./ProcessGFiles.h"


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
#include <QDesktopServices>

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <iomanip>
#include <ctime>
#include <sstream>

namespace fs = std::filesystem;








LibraryWindow::LibraryWindow(QWidget* parent) : QWidget(parent)
{
  this->setFixedSize(QSize(876, 600));
  ui.setupUi(this);

  //tagsWidget = new QListWidget(this);

  /* make Model selections update the tabs */
  connect(ui.listWidget, &QListWidget::currentItemChanged, this, &LibraryWindow::onModelSelectionChanged);
  connect(ui.generateReport, &QPushButton::pressed, this, &LibraryWindow::generateReport);
  // connect(ui.addTextInput, &QPushButton::pressed, this, &LibraryWindow::on_addTextInput_clicked);

}

void LibraryWindow::on_addTextInput_clicked(){

  // ui.verticalLayout->addWidget(new QTextEdit("hello"));
  // use setPlainText to add the command into the text box
}

LibraryWindow::~LibraryWindow()
{
}

void LibraryWindow::generateReport(){
  // file chooser
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  std::ostringstream oss;
  oss << std::put_time(&tm, "%d-%m-%Y_%H-%M-%S");
  auto time = oss.str();
  QString temp_dir_1= QFileDialog::getExistingDirectory(this, tr("Choose Directory to store Output"));
  std::cout << "directory to save: " << temp_dir_1.toStdString() << std::endl;
  std::string report_filepath = temp_dir_1.toStdString() + "/report_" + time + ".pdf";
  QPdfWriter pdfWriter(QString::fromStdString(report_filepath));
  
  // Set the resolution (optional)
  pdfWriter.setResolution(300);

  // Set the page size (A4, Letter, etc.)
  pdfWriter.setPageSize(QPageSize(QPageSize::A4));

  // Create a QPainter to draw on the QPdfWriter
  QPainter painter(&pdfWriter);

  // Set a font for the text
  QFont font("Helvetica", 18);
  painter.setFont(font);
  painter.drawText(750,200, "Cadventory");
  painter.setPen(QPen(Qt::black, 3));
  painter.drawLine(-200, 250, 2450, 250);
  painter.drawLine(-200, 3400, 2450, 3400);

  std::vector<std::string> test_library_models = library->getGeometry();
  QFont font_two("Helvetica", 6);
  painter.setFont(font_two);
  painter.drawText(300, 300, QString::fromStdString("Library: " + std::string(library->name())));

  // will need to adjust for windows, macos "local home" libraries

  std::string dir_slash = std::string(library->name()) + "/";
  int x = 325;
  int y = 350;
  painter.drawText(x, y, "Geometry");
  y+=25;

  for(const auto& str: geometryModel->stringList()){
    std::string cur_model = str.toStdString();
    y+=25;
    painter.drawText(x, y, QString::fromStdString(cur_model));
  }
  y+=50;
  painter.drawText(x,y, "Images");
  y+=25;
  for(const auto& str: imagesModel->stringList()){
    std::string cur_model = str.toStdString();
    y+=25;
    painter.drawText(x, y, QString::fromStdString(cur_model));
  }
  y+=50;
  painter.drawText(x,y,"Documents");
  y+=25;
  for(const auto& str: documentsModel->stringList()){
    std::string cur_model = str.toStdString();
    y+=25;
    painter.drawText(x, y, QString::fromStdString(cur_model));
  }

  painter.setFont(font);
  ProcessGFiles gFileProcessor; // just to call commands
  int num_file = 0;
  std::cout << "generating gist reports" << std::endl;
  painter.rotate(90);

  for(auto str: report){
    std::cout << "model selected: " << str << std::endl;
    std::string path_gist_output = temp_dir_1.toStdString() + "/" + std::to_string(num_file)+".png";
    std::string gist_command = "/home/anton/brlcad/build/bin/gist " + str + " -o " + path_gist_output;

    auto [output, error]  =  gFileProcessor.runCommand(gist_command);
    // End painting
    std::cout << "std output: " << output << std::endl;
    std::string png = temp_dir_1.toStdString() + "/" + std::to_string(num_file) + ".png";
    QString png_qstr = QString::fromStdString(png);
    QPixmap gist(png_qstr);
    bool status_newpage = pdfWriter.newPage();
    if(output.find("ERROR") == std::string::npos){
      painter.drawPixmap(0, -2408, gist);
    }else{
      painter.rotate(-90);
      painter.setFont(font);
      painter.drawText(100, 100, QString::fromStdString(str));
      painter.setFont(font_two);
      painter.drawText(100, 150, QString::fromStdString(output));
      painter.rotate(90);
    }
    num_file++;
  }
  painter.end();
  std::cout << "Report Generated" << std::endl;
  QDesktopServices::openUrl(QUrl(temp_dir_1));
  // open pdf after generation
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

  /* prepare MVC x for our list views */
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
  std::cout << "Seting current tags model" << std::endl;
  ui.currentTagsListView->setModel(currentTagsModel);
  std::cout << "Setting current properties model" << std::endl;
  ui.currentPropertiesListView->setModel(currentPropertiesModel);
  std::cout << "Set current properties model" << std::endl;

  /* load tags */
  std::cout << "Calling display tags for library: " << library->name() << std::endl;
  loadTags();
  std::cout << "Called display tags for library: " << library->name() << std::endl;

   std::string root_folder = fs::current_path().string()+"/previews/";  // Get the current root folder of the project

   for (const QString &path : geometryModel->stringList()) {
       // Check if the string ends with ".g"
       if (path.endsWith(".g", Qt::CaseInsensitive)) {


           QString filename = QFileInfo(path).fileName();
           std::cout << filename.toStdString() << std::endl;
           //QString filepath = ":/build/previews/"+filename;

           QString filepath = QString::fromStdString(root_folder);

           filepath.append(filename);
           filepath.chop(2);
           filepath.append(".png");
           std::cout << filepath.toStdString() << std::endl;
           // Print the filename without the extension

           QListWidgetItem *item = new QListWidgetItem(QIcon(filepath), filename);
           QList<QSize> availableSizes = item->icon().availableSizes();

           if (availableSizes.isEmpty()){
            delete item;
           }
           else{
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

   QStyledItemDelegate *delegate = new QStyledItemDelegate(this);
   ui.listWidgetPage->setItemDelegate(delegate);
}

void
LibraryWindow::on_allLibraries_clicked()
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
    return; // nada selected

  QString selectedDir = current->text();
  int modelId = library->model->hashModel(library->fullPath+"/"+selectedDir.toStdString());

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
  // library->model->printModel(modelId);
  currentPropertiesModel->setStringList(qModelProperties);

  /* retrieve full lists */
  // std::vector<std::string> allGeometry = library->getGeometry();
  // std::vector<std::string> allImages = library->getImages();
  // std::vector<std::string> allDocuments = library->getDocuments();
  // std::vector<std::string> allData = library->getData();

  /* filter and update the models for each category */
  // std::cout << "Updating models for directory: " << selectedDir.toStdString() << std::endl;
  // updateListModelForDirectory(geometryModel, allGeometry, selectedDir.toStdString());
  // std::cout << "Updated geometry model" << std::endl;
  // updateListModelForDirectory(imagesModel, allImages, selectedDir.toStdString());
  // std::cout << "Updated images model" << std::endl;
  // updateListModelForDirectory(documentsModel, allDocuments, selectedDir.toStdString());
  // std::cout << "Updated documents model" << std::endl;
  // updateListModelForDirectory(dataModel, allData, selectedDir.toStdString());
  // std::cout << "Updated data model" << std::endl;
}

// loads all tags from models from a directory ordered by prevalence
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

// void LibraryWindow::on_listWidgetPage_itemClicked(QListWidgetItem *item)
// {
//     ui.listWidgetPage->
// }


void LibraryWindow::on_pushButton_clicked()
{
    report = this->library->getModels();
    for(std::string& path : report){
        path = library->fullPath+"/"+path;
    }
}


void LibraryWindow::on_listWidgetPage_itemClicked(QListWidgetItem *item)
{
    std::string name = item->text().toStdString();

    int i = 0;
    for (const auto& file : report) {


        std::filesystem::path filePath(file);

        // Extracting the filename as a string
        std::string filename = filePath.filename().string();


        if(name == filename){

            report.erase(report.begin() + i);

              QFont font = item->font();
              font.setBold(false);
              item->setFont(font);
              std::cout << filename + " is unselected from report" << std::endl;

            return;
        }
        i++;
    }
    QFont font = item->font();
    font.setBold(true);
    item->setFont(font);
    report.push_back(this->library->fullPath+"/"+name);
      std::cout << this->library->fullPath+"/"+name << std::endl;
    std::cout << name + " is selected for report" << std::endl;



}

