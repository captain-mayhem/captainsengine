
#include "SQL.h"

SQL::SQL(std::string dbname){
  int res = sqlite3_open(dbname.c_str(), &db_);
}

SQL::~SQL(){
  sqlite3_close(db_);
}

std::string SQL::execute(std::string sql){
  char** query_result;
  char* err_msg;
  int rows;
  int cols;
  header_.clear();
  entries_.clear();
  int res = sqlite3_get_table(db_, sql.c_str(), &query_result, &rows, &cols, &err_msg);
  if (res != SQLITE_OK){
    sqlite3_free_table(query_result);
    return err_msg;
  }
  for (int i = 0; i < cols; ++i){
    header_.push_back(query_result[i]);
  }
  for (int i = 0; i < rows*cols; ++i){
    entries_.push_back(query_result[cols+i]);
  }
  sqlite3_free_table(query_result);
  return entries_[0];
}

std::string SQL::getResultString(std::string column, int row){
  int idx = -1;
  for (unsigned i = 0; i < header_.size(); ++i){
    if (header_[i] == column){
      idx = (int)i;
      break;
    }
  }
  if (idx == -1)
    return "";
  return entries_[row*header_.size()+idx];
}

int SQL::getResultInt(std::string column, int row){
  std::string res = getResultString(column, row);
  return atoi(res.c_str());
}
