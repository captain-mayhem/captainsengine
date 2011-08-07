#include "Textout.h"
#include "ScriptDefs.h"
#include "Engine.h"

Textout::Textout() : mEnabled(false), mText(NULL), mPos(0,0), mFont(1), mColor(Engine::instance()->getSettings()->infotextcolor){
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
  Vec2i pos(0,0);
  bool keepOnScreen = true;
  int depth = DEPTH_GAME_FONT;
  if (!mBoundRoom.empty()){
    RoomObject* ro = Engine::instance()->getRoom(mBoundRoom);
    if (ro == NULL)
      return;
    pos = ro->getScrollOffset();
    depth = ro->getAnimation()->getDepth()+DEPTH_BOUND_FONT;
    keepOnScreen = false;
  }
  Engine::instance()->getInterpreter()->executeImmediately(mText, false);
  std::string text = mText->stack().pop().getString();
  std::vector<Vec2i> breakinfo;
  Vec2i ext = Engine::instance()->getFontRenderer()->getTextExtent(text, mFont, breakinfo);
  Engine::instance()->getFontRenderer()->render(mPos.x-(keepOnScreen ? ext.x/2 : 0)+pos.x,mPos.y+pos.y, text, 
      depth, mFont, breakinfo, mColor, 0, keepOnScreen);
}
