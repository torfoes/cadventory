#ifndef DRIVEINFO_H
#define DRIVEINFO_H

#include <QStorageInfo>
#include <QWidget>

namespace Ui {
class DriveInfo;
}

/*!
 * \brief The DriveInfo class is used to display information about the current drive.
 */
class DriveInfo : public QWidget
{
    Q_OBJECT

public:
    explicit DriveInfo(QWidget *parent = nullptr);
    ~DriveInfo();
    void refreshDrive(const QString &path = QDir::currentPath());

private:
    Ui::DriveInfo *ui;
    QStorageInfo *info;
};

#endif // DRIVEINFO_H
