#include "previews.h"
#include "ui_previews.h"

Previews::Previews(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Previews)
{
    ui->setupUi(this);

}

Previews::~Previews()
{
    delete ui;
}
