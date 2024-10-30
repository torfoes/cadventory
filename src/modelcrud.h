#ifndef MODELCRUD_H
#define MODELCRUD_H

#include <QDialog>

namespace Ui {
class modelcrud;
}

class modelcrud : public QDialog
{
    Q_OBJECT

public:
    explicit modelcrud(QWidget *parent = nullptr);
    ~modelcrud();

private:
    Ui::modelcrud *ui;
};

#endif // MODELCRUD_H
