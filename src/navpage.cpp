#include "navpage.h"
#include "ui_navpage.h"

#include <QColumnView>
#include <QLayout>
#include <QListView>
#include <QProcess>
#include <QTableView>
#include <QTreeView>

Navpage::Navpage(QFileSystemModel *model, LibraryWindow *root, QWidget *parent)
    : QWidget(parent), ui(new Ui::Navpage), root(root), model(model)
{
    ui->setupUi(this);

    current_dir = nullptr;

    QListView *view = new QListView();
    view->setWordWrap(true);
    view->setViewMode(QListView::IconMode);
    view->setIconSize(QSize(48, 48));
    view->setGridSize(QSize(128, 72));
    view->setUniformItemSizes(true);
    view->setMovement(QListView::Static);
    view->setResizeMode(QListView::Adjust);
    view->setLayoutMode(QListView::Batched);
    view->setBatchSize(10);
    dirView = static_cast<QAbstractItemView *>(view);
    driveInfo = new DriveInfo(this);
    itemInfo = nullptr;
    set_up_dirview();
    ui->verticalLayout->addWidget(dirView);
    ui->verticalLayout->addWidget(driveInfo);

    connect(dirView, &QAbstractItemView::activated, this, &Navpage::dirView_open_item);
    connect(dirView, &QAbstractItemView::clicked, this, &Navpage::dirView_show_iteminfo);
}

Navpage::~Navpage()
{
    delete ui;
    delete dirView;
    delete itemInfo;
    delete driveInfo;
    delete current_dir;
}

int Navpage::change_dir(QString new_dir)
{
    if (current_dir) {
        delete current_dir;
    }
    current_dir = new QDir(new_dir);
    auto index = model->index(new_dir);
    qDebug() << index;
    dirView->setRootIndex(index);
    driveInfo->refreshDrive(new_dir);
    return 0;
}

void Navpage::set_up_dirview()
{
    if (dirView->objectName().isEmpty())
        dirView->setObjectName("dirView");
    dirView->resize(0, 0);
    dirView->adjustSize();
    QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    sizePolicy.setHorizontalStretch(1);
    sizePolicy.setVerticalStretch(1);
    //	sizePolicy.setHeightForWidth(dirView->sizePolicy().hasHeightForWidth());
    dirView->setSizePolicy(sizePolicy);
    dirView->setMinimumSize(QSize(400, 300));
    dirView->setAcceptDrops(true);

    dirView->setModel(model);
    qDebug() << dirView;
    auto index = model->index(QDir::currentPath());
    qDebug() << index;
    dirView->setRootIndex(index);
}

void Navpage::dirView_open_item(const QModelIndex &index)
{
    qDebug() << index;
    if (root->check_n_change_dir(model->filePath(index), LibraryWindow::CDSource::Navpage, true))
        change_dir(model->filePath(index));
    else {
        qDebug() << "Should open the file here.";
        QProcess launcher;
        QString program;
        QStringList arguments;

#if defined(Q_OS_LINUX)
        program = "xdg-open";
        arguments << model->filePath(index);
#elif defined(Q_OS_WIN)
        program = "cmd.exe";
        arguments << "/C"
                  << "start"
                  << "" << model->filePath(index);
#elif defined(Q_OS_MACOS)
        program = "open";
        arguments << model->filePath(index);
#endif

        launcher.setProgram(program);
        launcher.setArguments(arguments);
        qDebug() << "Process started" << launcher.startDetached();
    }
}

QStringList Navpage::get_selection()
{
    QModelIndexList list = dirView->selectionModel()->selectedIndexes();
    QStringList path_list;
    foreach (const QModelIndex &index, list) {
        path_list.append(model->filePath(index));
    }
    qDebug() << path_list.join(",");
    return path_list;
}

void Navpage::dirView_show_iteminfo(const QModelIndex &index)
{
    QFileInfo fileInfo = model->fileInfo(index);
    if (itemInfo) {
        ui->verticalLayout->removeWidget(itemInfo);
        delete itemInfo;
    }
    itemInfo = new DirItemInfo(&fileInfo, this);
    itemInfo->refresh();
    ui->verticalLayout->addWidget(itemInfo);
    qDebug() << index;
}
