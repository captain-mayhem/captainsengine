#include "Textout.h"
#include "ScriptDefs.h"
#include "Engine.h"

Textout::Textout() : mEnabled(false), mText(NULL), mPos(0,0), mFont(1), mColor(){
}

Textout::~Textout(){
  delete mText;
}

void Textout::setText(ExecutionContext* text){
  delete mText;
  mText = new ExecutionContext(*text);
}

void Textout::setFont(int fontid){
  mFont = fontid;
  Engine::instance()->getFontRenderer()->loadFont(fontid);
}

void Textout::render(){
  if (!mEnabled)
    return;
  Engine::instance()->getInterpreter()->executeImmediately(mText, false);
  std::string text = mText->stack().pop().getString();
  std::vector<Vec2i> breakinfo;
  Vec2i ext = Engine::instance()->getFontRenderer()->getTextExtent(text, mFont, breakinfo);
  Engine::instance()->getFontRenderer()->render(mPos.x-ext.x/2,mPos.y/*-ext.y*/, text, 
      DEPTH_GAME_FONT, mFont, breakinfo, mColor, 0);
}
