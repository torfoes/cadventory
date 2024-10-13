#ifndef MYCLASS_H
#define MYCLASS_H


#include <QWidget>
#include <QMessageBox>
#include <QFileSystemWatcher>
#include <iostream>
class MyClass : public QWidget
{
    Q_OBJECT

public:
    MyClass(QWidget* parent=0)
        :QWidget(parent){}

    ~MyClass(){}

public slots:
    void showModified(const QString& str)
    {
        // Q_UNUSED(str)
        QMessageBox::information(this, "Directory Modified", "Your Directory is modified");
        std::cout << "modified file: " << str.toStdString() << std::endl;
        
    }
private:

    // QFileSystemWatcher watcher;
};


#endif
