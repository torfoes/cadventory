
#include "db_interface.h"

db_interface::db_interface() {

    // init database
    // path to sqlite database in question
    QString path = "test.db";

    QSqlDatabase m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(path);

    if (!m_db.open())
    {
        std::cout << "Error: connection with database failed" <<  std::endl;
        std::cout << m_db.lastError().text().toStdString() << std::endl;
    }
    else
    {
        std::cout << "Database: connection ok" <<  std::endl;
    }
}


