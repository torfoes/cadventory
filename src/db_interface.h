#ifndef DB_INTERFACE_H
#define DB_INTERFACE_H

#include <iostream>
#include <string>
#include <QtSql>

class db_interface
{
public:
    db_interface();
private:
    std::string file_path;
    QSqlDatabase* db;

};

#endif // DB_INTERFACE_H
