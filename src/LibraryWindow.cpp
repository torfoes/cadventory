
#include "./LibraryWindow.h"
#include "navpage.h"
#include "utils.h"

#include <QClipboard>
#include <QDebug>
#include <QInputDialog>
#include <QLineEdit>
#include <QObject>
#include <QTabWidget>
#include <QTextEdit>

#include <QStringListModel>
#include <QPixmap>
#include <QMessageBox>


LibraryWindow::LibraryWindow(QWidget* parent) : QWidget(parent)
{
  this->setFixedSize(QSize(640, 480));
  ui.setupUi(this);

  /* make Model selections update the tabs */
  connect(ui.listWidget, &QListWidget::currentItemChanged, this, &LibraryWindow::onModelSelectionChanged);

  QPixmap pix("/Users/quhuigang/wkspaces/Capstone_CAD/CADventory/src/die.png");
  int w = ui.Example->width(), h = ui.Example->height();
  ui.Example->setPixmap(pix.scaled(w,h,Qt::KeepAspectRatio));

  FSmodel = new QFileSystemModel();
  //	model->setRootPath(QDir::currentPath());
  qDebug() << "setRootPath" << QDir::rootPath();
  FSmodel->setRootPath(QDir::rootPath());


  // Navpage *newpage = new Navpage(FSmodel, this);
  // newpage->change_dir(QDir::homePath());

  // ui.tabWidget->setCurrentWidget(newpage);

}


LibraryWindow::~LibraryWindow()
{
}


void
LibraryWindow::loadFromLibrary(Library* _library)
{
  this->library = _library;
  this->setWindowTitle(library->name() + QString(" Library"));
  this->ui.currentLibrary->setText(library->name());
  QListView *image = ui.imagesListView;
  image->setWordWrap(true);
  image->setViewMode(QListView::IconMode);
  image->setIconSize(QSize(48, 48));
  image->setGridSize(QSize(128, 72));
  image->setUniformItemSizes(true);
  image->setMovement(QListView::Static);
  image->setResizeMode(QListView::Adjust);
  image->setLayoutMode(QListView::Batched);
  image->setBatchSize(10);


  /* start fresh */
  ui.listWidget->clear();

  /* populate Models listing */
  auto modelDirs = library->getModels();
  for (const auto& dir : modelDirs) {
    QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(dir));
    ui.listWidget->addItem(item);
  }

  /* prepare MVC model for our list views */
  auto populateModel = [this](QStringListModel* model, const std::vector<std::string>& fullPaths) {
    QStringList list;
    std::string base = std::string(library->path()) + "/";
    for (const auto& fullPath : fullPaths) {
      std::string relativePath = fullPath.substr(base.length());
      list << QString::fromStdString(relativePath);
    }
    model->setStringList(list);
  };

  geometryModel = new QStringListModel(this);
  imagesModel = new QStringListModel(this);
  documentsModel = new QStringListModel(this);
  dataModel = new QStringListModel(this);

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
}


void
LibraryWindow::on_allLibraries_clicked()
{
  this->hide();
    Main->show();
}


void
LibraryWindow::updateListModelForDirectory(QStringListModel* model, const std::vector<std::string>& allItems, const std::string& directory)
{
  QStringList filteredItems;
  std::string base = library->path() + (directory == "." ? "" : "/" + directory);
  for (const auto& item : allItems) {
    if (item.find(base) == 0) { // item starts with base path
      std::string relativePath = item.substr(base.length() + 1); // +1 to skip leading slash
      filteredItems << QString::fromStdString(relativePath);
    }
  }
  model->setStringList(filteredItems);
}


void
LibraryWindow::onModelSelectionChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
  if (!current)
    return; // nada selected

  QString selectedDir = current->text();

  /* retrieve full lists */
  std::vector<std::string> allGeometry = library->getGeometry();
  std::vector<std::string> allImages = library->getImages();
  std::vector<std::string> allDocuments = library->getDocuments();
  std::vector<std::string> allData = library->getData();

  /* filter and update the models for each category */
  updateListModelForDirectory(geometryModel, allGeometry, selectedDir.toStdString());
  updateListModelForDirectory(imagesModel, allImages, selectedDir.toStdString());
  updateListModelForDirectory(documentsModel, allDocuments, selectedDir.toStdString());
  updateListModelForDirectory(dataModel, allData, selectedDir.toStdString());
}



bool LibraryWindow::check_n_change_dir(const QString &path, CDSource source, bool suppress_warning)
{
    QDir *dir = new QDir(path);
    QString message = "This is not a folder.";
    if (dir->exists()) {
        QDir::setCurrent(path);

        if (source != CDSource::Navbar) {
            ui.addressBar->setText(dir->absolutePath());
            ui.addressBar->update();
        }
        if (source != CDSource::Tabchange) {
            Navpage *currentpage = static_cast<Navpage *>(ui.tabWidget_2->currentWidget());
            if (currentpage != nullptr) {
                currentpage->change_dir(dir->absolutePath());
                ui.tabWidget_2->setTabText(ui.tabWidget_2->currentIndex(), dir->dirName());
                ui.tabWidget_2->update();
            }
        }

        visitedPaths.push_back(path);
        qDebug() << visitedPaths.size();
        delete dir;
        return true;
    } else {
        if (!suppress_warning)
            show_warning(message);
        delete dir;
        return false;
    }
}


void LibraryWindow::show_warning(const QString &message)
{
    QMessageBox *alert = new QMessageBox();
    alert->setText(message);
    alert->setIcon(QMessageBox::Warning);
    alert->setWindowIcon(windowIcon());
    alert->exec();
    delete alert;
}


int LibraryWindow::add_page_to_tabpanel(QString dir, const QString &label)
{
    Navpage *newpage = new Navpage(FSmodel, this);
    newpage->change_dir(dir);
    ui.tabWidget_2->addTab(newpage, label);
    visitedPaths.push_back(dir);
    return 0;
}


void LibraryWindow::on_actionNew_tab_triggered()
{
    add_page_to_tabpanel(QDir::homePath(), "Home");
}


