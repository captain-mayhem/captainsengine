#include "Textout.h"
#include "ScriptDefs.h"
#include "Engine.h"
#include <system/allocation.h>

using namespace adv;

Textout::Textout() : Object2D(1, Vec2i(), Vec2i(), "textout"), mEnabled(false), mText(NULL), mFont(1), mColor(Engine::instance()->getSettings()->infotextcolor),
  mAlignment(ALGN_LEFT), mTimeShown(0), mFadingOut(false){
    FontRenderer::Font* fnt = Engine::instance()->getFontRenderer()->getFont(mFont);
    mFadingTime = fnt ? fnt->getFading() : 0;
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
  mFadingTime = Engine::instance()->getFontRenderer()->getFont(fontid)->getFading();
}

void Textout::render(unsigned time){
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
  mText->reset(false, true);
  std::string text;
  if (sd.isNumber()){
    char tmp[32];
    if (sd.isFloat())
      sprintf(tmp, "%.2f", sd.getFloat());
    else
      sprintf(tmp, "%i", sd.getInt());
    text = tmp;
  }
  else
    text = sd.getString();
  std::vector<Vec2i> breakinfo;
  Vec2i ext = Engine::instance()->getFontRenderer()->getTextExtent(text, mFont, breakinfo, Engine::instance()->getData()->getProjectSettings()->resolution.x);
  FontRenderer::String* result = Engine::instance()->getFontRenderer()->render(mPos.x+pos.x,mPos.y+pos.y, text, 
      depth, mFont, breakinfo, mColor, 0, false, mAlignment);
  result->setBoundRoom(boundRoom);
  //text fading
  unsigned char opacity = 255;
  if (mFadingTime > 0){
    float factor;
    if (mTimeShown < mFadingTime)
      factor = mTimeShown/(float)mFadingTime;
    else
      factor = 1.0f;//mDisplayTime/(float)mFadingTime;
    if (factor > 1.0f)
      factor = 1.0f;
    opacity = (unsigned char)(factor*255);
  }
  result->setOpacity(opacity);
  mTimeShown += mFadingOut ? -(int)time : time;
  if (mFadingOut && mTimeShown <= 0){
    mFadingOut = false;
    mEnabled = false;
    mTimeShown = 0;
  }
}

void Textout::save(std::ostream& out){
  out << mEnabled << " " << mPos.x << " " << mPos.y << " " << mFont << "\n";
  out << mColor << " ";
  out << (mBoundRoom.empty() ? "none" : mBoundRoom);
  out << "\n";
  out << (int)mAlignment << "\n";
  if (mText){
    out << 1 << " ";
    mText->save(out);
  }
  else
    out << 0 << "\n";
}

void Textout::load(std::istream& in, int version){
  in >> mEnabled >> mPos.x >> mPos.y >> mFont;
  in >> mColor;
  in >> mBoundRoom;
  if (mBoundRoom == "none")
    mBoundRoom = "";
  int tmp;
  in >> tmp; mAlignment = (Alignment)tmp;
  Engine::instance()->getFontRenderer()->loadFont(mFont);
  bool hasText = true;
  if (version >= 2)
    in >> hasText;
  if (hasText)
    mText = new ExecutionContext(in);
  else
    mText = NULL;
}

void Textout::setEnabled(bool enabled){
  if (!mEnabled)
    mFadingTime = 0;
  if (mFadingOut & enabled)
    mFadingOut = false;
  if (mEnabled && !enabled && mFadingTime > 0){
    mFadingOut = true;
    mTimeShown = mFadingTime;
  }
  else
    mEnabled = enabled;
}
