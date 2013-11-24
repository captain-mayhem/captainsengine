#ifndef TEXTOUT_H
#define TEXTOUT_H

#include "Vector.h"
#include "BlitObjects.h"
#include "String.h"

namespace adv{

class ExecutionContext;

class Textout{
public:
  enum Alignment{
    LEFT,
    CENTER,
    RIGHT,
  };
  Textout();
  ~Textout();
  void render();
  void setEnabled(bool enabled) {mEnabled = enabled;}
  void setText(ExecutionContext* text);
  Vec2i& getPos() {return mPos;}
  void setFont(int fontid);
  Color& getColor() {return mColor;}
  void setRoom(const String& room) {mBoundRoom = room;}
  void save(std::ostream& out);
  void load(std::istream& in);
  void setAlignment(Alignment alignment) {mAlignment = alignment;}
protected:
  bool mEnabled;
  ExecutionContext* mText;
  Vec2i mPos;
  int mFont;
  Color mColor;
  String mBoundRoom;
  Alignment mAlignment;
};

}

#endif