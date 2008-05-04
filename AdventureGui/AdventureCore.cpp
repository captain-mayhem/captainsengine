#include <iostream>
#include <sstream>
#include "system/engine.h"
#include "renderer/renderer.h"
#include "renderer/font.h"
#include "renderer/forms.h"
#include "math/vector.h"
#include "gui/messagebox.h"
#include "gui/dropdown.h"
#include "input/keyboard.h"
#include "input/mouse.h"

#include "AdventureCore.h"
#include "SQL.h"

using std::cerr;
using namespace Graphics;
using namespace System;

#define CURSOR_X 10
#define CURSOR_Y 740
#define RIGHT_INDENT 300
#define CHAR_WIDTH 10
#define MAX_CHARS ((SCREENWIDTH-RIGHT_INDENT)/CHAR_WIDTH-1)
#define LINE_SPACING 20

AdventureCore::AdventureCore(SQL* sql){
  m_sql = sql;
  cursor = Vector2D(CURSOR_X, CURSOR_Y);
  second_cursor = Vector2D(1024-RIGHT_INDENT+CURSOR_X, CURSOR_Y);
  Graphics::Font* fnt = System::Engine::instance()->getFont(0);
  fnt->setColor(Graphics::Color(0,1,0,1));
  chid_ = "0";
}

AdventureCore::~AdventureCore(){

}

void AdventureCore::displayText(std::string txt){
  //split up into lines
  std::istringstream str(txt);
  unsigned maxchars = (SCREENWIDTH-CURSOR_X-RIGHT_INDENT)/CHAR_WIDTH-1;
  string word;
  while (str >> word){
    //transform word (remove umlaute and ss)
    word = transform(word);
    //the word does not fit into the current line
    printWord(word);
  }
}

void AdventureCore::printWord(std::string txt){
  //the word does not fit into the current line
  if (cursor.x/CHAR_WIDTH + txt.size() > MAX_CHARS){
    newline();
  }
  Graphics::Font* fnt = System::Engine::instance()->getFont(0);
  txt += " ";
  fnt->print(cursor.x, cursor.y, txt.c_str(), 0);
  cursor.x += txt.size()*CHAR_WIDTH;
}

void AdventureCore::newline(){
  cursor.y -= LINE_SPACING;
  cursor.x = CURSOR_X;
}

void AdventureCore::printLine(std::string txt){
  Graphics::Font* fnt = System::Engine::instance()->getFont(0);
  fnt->print(cursor.x,cursor.y, txt.c_str(), 0);
  cursor.y -= LINE_SPACING;
  cursor.x = CURSOR_X;
}

std::string AdventureCore::transform(std::string txt){
  string ret;
  for (unsigned i = 0; i < txt.size(); ++i){
    char letter = txt[i];
    switch(letter){
        case 'ä':
          ret.append("ae"); break;
        case 'ü':
          ret.append("ue"); break;
        case 'ö':
          ret.append("oe"); break;
        case 'Ä':
          ret.append("Ae"); break;
        case 'Ö':
          ret.append("Oe"); break;
        case 'Ü':
          ret.append("Ue"); break;
        case 'ß':
          ret.append("ss"); break;
        default:
          ret.append(1, letter);
    }
  }
  return ret;
}

//transforms a utf8 string into an ascii string
std::string AdventureCore::transformUtf8(std::string txt){
  wchar_t tmp[1024];
  unsigned size = MultiByteToWideChar(CP_UTF8, 0, txt.c_str(), txt.size(), tmp, 1024);
  string ret;
  for (unsigned i = 0; i < size; ++i){
    wchar_t letter = tmp[i];
    ret.append(1, (char)letter);
  }
  return ret;
}

void AdventureCore::clearDisplay(){
  cursor = Vector2D(CURSOR_X, CURSOR_Y);
}

void AdventureCore::showWays(){
  std::string rid = m_sql->execute("SELECT curr_room FROM characters where chid="+chid_+";");
  m_sql->execute("SELECT rid, name FROM room_connections JOIN rooms ON room_connections.too = rooms.rid WHERE room_connections.frm ="+rid+";");
  for (int i = 0; i < m_sql->getNumRows(); ++i){
    displayOption(transformUtf8(m_sql->getResultString("name", i)));
  }
}

//displays a choosable option
void AdventureCore::displayOption(std::string txt){
  Graphics::Font* fnt = System::Engine::instance()->getFont(1);
  fnt->setColor(Graphics::Color(0,0,1,1));
  fnt->print(second_cursor.x, second_cursor.y, transform(txt).c_str(), 0);
  second_cursor.y -= LINE_SPACING;
}
