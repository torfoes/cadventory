#ifndef PREVIEWS_H
#define PREVIEWS_H

#include <QWidget>

namespace Ui {
class Previews;
}

class Previews : public QWidget
{
    Q_OBJECT

public:
    explicit Previews(QWidget *parent = nullptr);
    ~Previews();

private:
    Ui::Previews *ui;
};

#endif // PREVIEWS_H
