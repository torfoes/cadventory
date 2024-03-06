#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <sqlite3.h>


class Model {
public:
  Model(const std::string& path);
  ~Model();

  bool createTable();
  bool insertModel(const std::string& shortName, const std::string& file = "", const std::string& overrides = "");

private:
  sqlite3* db;
  std::string dbPath;

  bool executeSQL(const std::string& sql);
};


#endif // MODEL_H
