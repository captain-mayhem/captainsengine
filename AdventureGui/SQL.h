//  ==================================================================
// |                         Hero Engine                              |
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
  std::string execute(std::string sql);
  std::string getResultString(std::string column, int row);
  int getResultInt(std::string column, int row);
  int getNumRows() {return entries_.size()/header_.size();}
private:
  sqlite3* db_;
  std::vector<std::string> header_;
  std::vector<std::string> entries_;
};