#include "Textout.h"
#include "ScriptDefs.h"
#include "Engine.h"

using namespace adv;

Textout::Textout() : mEnabled(false), mText(NULL), mPos(0,0), mFont(1), mColor(Engine::instance()->getSettings()->infotextcolor),
  mAlignment(LEFT){
}

Textout::~Textout(){
  mText->unref();
}

void Textout::setText(ExecutionContext* text){
  mText->unref();
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
  //bool keepOnScreen = true;
  int depth = DEPTH_GAME_FONT;
  RoomObject* boundRoom = NULL;
  if (!mBoundRoom.empty()){
    boundRoom = Engine::instance()->getRoom(mBoundRoom);
    if (boundRoom == NULL)
      return;
    pos = boundRoom->getScrollOffset()+boundRoom->getPosition();
    depth = boundRoom->getDepth()+DEPTH_BOUND_FONT;
    //keepOnScreen = false;
  }
  Engine::instance()->getInterpreter()->executeImmediately(mText, false);
  StackData sd = mText->stack().pop();
  std::string text;
  if (sd.isNumber()){
    char tmp[32];
    sprintf(tmp, "%i", sd.getInt());
    text = tmp;
  }
  else
    text = sd.getString();
  std::vector<Vec2i> breakinfo;
  Vec2i ext = Engine::instance()->getFontRenderer()->getTextExtent(text, mFont, breakinfo, Engine::instance()->getData()->getProjectSettings()->resolution.x);
  int alignoffset = 0;
  if (mAlignment == CENTER)
    alignoffset = ext.x/2;
  else if (mAlignment == RIGHT)
    alignoffset = ext.x;
  FontRenderer::String* result = Engine::instance()->getFontRenderer()->render(mPos.x/*-(keepOnScreen ? ext.x/2 : 0)*/+pos.x-alignoffset,mPos.y+pos.y, text, 
      depth, mFont, breakinfo, mColor, 0, false);
  result->setBoundRoom(boundRoom);
}

void Textout::save(std::ostream& out){
  out << mEnabled << " " << mPos.x << " " << mPos.y << " " << mFont << "\n";
  out << mColor << " ";
  out << (mBoundRoom.empty() ? "none" : mBoundRoom);
  out << "\n";
  out << (int)mAlignment << "\n";
  mText->save(out);
}

void Textout::load(std::istream& in){
  in >> mEnabled >> mPos.x >> mPos.y >> mFont;
  in >> mColor;
  in >> mBoundRoom;
  if (mBoundRoom == "none")
    mBoundRoom = "";
  int tmp;
  in >> tmp; mAlignment = (Alignment)tmp;
  Engine::instance()->getFontRenderer()->loadFont(mFont);
  mText = new ExecutionContext(in);
}
