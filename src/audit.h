#ifndef AUDIT_H
#define AUDIT_H

#include <vector>
#include <string>
#include <QtSql>
#include <iostream>
#include "Library.h"
#include <QCryptographicHash>

class Audit
{
public:
    Audit(QSqlDatabase& db, std::vector<Library*> libraries);

    std::string calculateCheckSum(std::string& file);

    // calculate checksums of things in library
    std::vector<std::pair<std::string, std::string>> checksumsAll();

    // insert into database function
    void insertFileChecksum(std::string &path, std::string &checksum, std::string &lib_name);

    // db verification routine
    void checkFiles();


private:
    QSqlDatabase db;
    std::vector<Library*> libraries;
};

#endif // AUDIT_H
