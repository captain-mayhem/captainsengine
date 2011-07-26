#ifndef TEXTOUT_H
#define TEXTOUT_H

#include "Vector.h"
#include "BlitObjects.h"

class ExecutionContext;

class Textout{
public:
  Textout();
  ~Textout();
  void render();
  void setEnabled(bool enabled) {mEnabled = enabled;}
  void setText(ExecutionContext* text);
  Vec2i& getPos() {return mPos;}
  void setFont(int fontid);
  Color& getColor() {return mColor;}
protected:
  bool mEnabled;
  ExecutionContext* mText;
  Vec2i mPos;
  int mFont;
  Color mColor;
};

#endif