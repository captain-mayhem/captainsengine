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

#include "math/vector.h"

namespace Graphics{
  class Font;
}
class SQL;

class AdventureCore{
public:
  AdventureCore(SQL* sql);
  ~AdventureCore();
  //displays arbitrary text
  void displayText(std::string txt);
  //displays a choosable option
  void displayOption(std::string txt);
  //shows all ways that can be walked
  void showWays();
  //deletes the text
  void clearDisplay();
  //goto a room
  void gotoRoom(int roomid);
private:
  //transforms a string into one without special characters
  std::string transform(std::string txt);
  //transforms a utf8 string into an ascii string
  std::string transformUtf8(std::string txt);
  //prints a text
  void printWord(std::string txt);
  //prints a line
  void printLine(std::string txt);
  //make a newline
  void newline();
  
  //a cursor position
  Vector2D cursor;
  //a second cursor position
  Vector2D second_cursor;
  //a SQL connection
  SQL* m_sql;
  //the character id
  std::string chid_;
};