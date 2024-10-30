#include "modelcrud.h"
#include "ui_modelcrud.h"

modelcrud::modelcrud(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::modelcrud)
{
    ui->setupUi(this);
}

modelcrud::~modelcrud()
{
    delete ui;
}
