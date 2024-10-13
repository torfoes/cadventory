#ifndef AUDITLOGWINDOW_H
#define AUDITLOGWINDOW_H

#include <QWidget>
#include <iostream>
#include <QtSql>
namespace Ui {
class AuditLogWindow;
}

class AuditLogWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AuditLogWindow(QWidget *parent = nullptr);
    void testFunction();
    ~AuditLogWindow();

private:
    Ui::AuditLogWindow *ui;
};

#endif // AUDITLOGWINDOW_H
