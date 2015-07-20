
#include "SQL.h"

SQL::SQL(std::string dbname){
  int res = sqlite3_open(dbname.c_str(), &db_);
}

SQL::~SQL(){
  sqlite3_close(db_);
}

int SQL::execute(const char* sql){
  char** query_result;
  int rows;
  int cols;
  header_.clear();
  entries_.clear();
  int res = sqlite3_get_table(db_, sql, &query_result, &rows, &cols, &error_msg_);
  if (res != SQLITE_OK){
    sqlite3_free_table(query_result);
    return -1;
  }
  for (int i = 0; i < cols; ++i){
    header_.push_back(query_result[i]);
  }
  for (int i = 0; i < rows*cols; ++i){
    if (query_result[cols+i] == NULL)
      entries_.push_back("");
    else
      entries_.push_back(query_result[cols+i]);
  }
  sqlite3_free_table(query_result);
  if (entries_.empty())
    return 0;
  return rows;
}

std::string SQL::getResultString(int column, int row){
  return entries_[row*header_.size()+column];
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

int SQL::getResultInt(int column, int row){
  std::string res = getResultString(column, row);
  return atoi(res.c_str());
}

int SQL::getResultInt(std::string column, int row){
  std::string res = getResultString(column, row);
  return atoi(res.c_str());
}

int SQL::getLastID(){
  return (int)sqlite3_last_insert_rowid(db_);
}
