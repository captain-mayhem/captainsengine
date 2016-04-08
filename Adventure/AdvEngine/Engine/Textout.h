#ifndef TEXTOUT_H
#define TEXTOUT_H

#include "Vector.h"
#include "BlitObjects.h"
#include "String.h"
#include "EngineObjects.h"

namespace adv{

class ExecutionContext;

class Textout : public Object2D{
public:
  Textout();
  ~Textout();
  void render(unsigned time);
  void setEnabled(bool enabled);
  void setText(ExecutionContext* text);
  Vec2i& getPos() {return mPos;}
  void setFont(int fontid);
  Color& getColor() {return mColor;}
  void setRoom(const String& room) {mBoundRoom = room;}
  void save(std::ostream& out);
  void load(std::istream& in);
  void setAlignment(Alignment alignment) {mAlignment = alignment;}
  String getBoundRoom() {return mBoundRoom;}
protected:
  bool mEnabled;
  ExecutionContext* mText;
  //Vec2i mPos;
  int mFont;
  Color mColor;
  String mBoundRoom;
  Alignment mAlignment;
  int mFadingTime;
  int mTimeShown;
  bool mFadingOut;
};

}

#endif