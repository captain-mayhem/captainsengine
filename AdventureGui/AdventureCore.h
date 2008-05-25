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

#include "math/vector.h"

#define CURSOR_X 10
#define CURSOR_Y 740
#define RIGHT_INDENT 300
#define CHAR_WIDTH 10
#define MAX_CHARS ((SCREENWIDTH-RIGHT_INDENT)/CHAR_WIDTH-1)
#define LINE_SPACING 20
#define SECOND_CURSOR_X SCREENWIDTH-RIGHT_INDENT+CURSOR_X
#define SECOND_CURSOR_Y CURSOR_Y

namespace Graphics{
  class Font;
}
class SQL;

class AdventureCore{
public:

  enum Action{
    None,
    Goto,
  };

  enum Opcode{
    none,
    textout,
    increment,
  };

  AdventureCore(SQL* sql);
  ~AdventureCore();
  //performs an action from the GUI
  void performAction(unsigned lineidx);
  //displays arbitrary text
  void displayText(std::string txt);
  //displays a choosable option
  void displayOption(Action act, std::string id, std::string txt);
  //shows all ways that can be walked
  void showWays();
  //deletes the text
  void clearDisplay();
  //deletes the second text
  void clearSecondDisplay();
  //goto a room
  void gotoRoom(std::string roomid);
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
  //interpret opcodess
  void interpret(Opcode opcode, std::string argument1, std::string argument2);
  
  //a cursor position
  Vector2D cursor;
  //a second cursor position
  Vector2D second_cursor;
  //a SQL connection
  SQL* m_sql;
  //the character id
  std::string chid_;
  //the current Action
  Action action_;
  //the ids for the actions
  std::vector<std::string> actionids_;
};