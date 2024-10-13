#include "auditlogwindow.h"
#include "ui_auditlogwindow.h"
// #include "db_interface.h"
AuditLogWindow::AuditLogWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AuditLogWindow)
{
    ui->setupUi(this);
    connect(ui->submitButton, &QPushButton::released, this, &AuditLogWindow::testFunction);
}

void AuditLogWindow::testFunction(){
    std::cout << "hello" << std::endl;
    // db_interface x;
}

AuditLogWindow::~AuditLogWindow()
{

}
