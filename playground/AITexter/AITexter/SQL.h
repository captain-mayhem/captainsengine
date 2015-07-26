//  ==================================================================
// |                         Captains Engine                          |
// |------------------------------------------------------------------|
// |             The simple but powerful game engine                  |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// | 2008-03-25                                                       |
// |------------------------------------------------------------------|
// | $Id$:                                                    |
//  ==================================================================

#include <string>
#include <vector>

#include "sqlite3.h"

class SQL{
public:
  SQL(std::string dbname);
  ~SQL();
  int execute(const char* sql);
  std::string getResultString(std::string column, int row);
  std::string getResultString(int column, int row);
  int getResultInt(std::string column, int row);
  int getResultInt(int column, int row);
  int getNumRows() {if (header_.size() == 0) return 0; return entries_.size()/header_.size();}
  int getNumCols() {if (header_.size() == 0) return 0; return header_.size();}
  int getLastID();
  const char* getError() {return error_msg_;}
private:
  sqlite3* db_;
  std::vector<std::string> header_;
  std::vector<std::string> entries_;
  char* error_msg_;
};