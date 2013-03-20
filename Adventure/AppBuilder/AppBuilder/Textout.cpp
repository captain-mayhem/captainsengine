#include "Textout.h"
#include "ScriptDefs.h"
#include "Engine.h"

using namespace adv;

Textout::Textout() : mEnabled(false), mText(NULL), mPos(0,0), mFont(1), mColor(Engine::instance()->getSettings()->infotextcolor){
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
  FontRenderer::String* result = Engine::instance()->getFontRenderer()->render(mPos.x/*-(keepOnScreen ? ext.x/2 : 0)*/+pos.x,mPos.y+pos.y, text, 
      depth, mFont, breakinfo, mColor, 0, false);
  result->setBoundRoom(boundRoom);
}

void Textout::save(std::ostream& out){
  out << mEnabled << " " << mPos.x << " " << mPos.y << " " << mFont << " ";
  out << mColor.r << " " << mColor.g << " " << mColor.b << " " << mColor.a << " ";
  out << (mBoundRoom.empty() ? "none" : mBoundRoom);
  out << "\n";
  mText->save(out);
}

void Textout::load(std::istream& in){
  in >> mEnabled >> mPos.x >> mPos.y >> mFont;
  in >> mColor.r >> mColor.g >> mColor.b >> mColor.a;
  in >> mBoundRoom;
  if (mBoundRoom == "none")
    mBoundRoom = "";
  Engine::instance()->getFontRenderer()->loadFont(mFont);
  mText = new ExecutionContext(in);
}
