#include "EngineObjects.h"
#include "Engine.h"
#include "SaveStateProvider.h"
#include "Inventory.h"
#include "ScriptFunc.h"
#include "Sound.h"

BlitGroup::BlitGroup(std::vector<std::string> textures, std::vector<Vec2i> offsets, int depth){
  for (unsigned l = (unsigned)textures.size(); l > 0; --l){
    BlitObject* obj = new BlitObject(textures[l-1], depth, offsets[l-1]);
    mBlits.push_back(obj);
  }
}

BlitGroup::BlitGroup(const std::string& texture, const Vec2i& offset, int depth){
  BlitObject* obj = new BlitObject(texture, depth, offset);
  mBlits.push_back(obj);
}

BlitGroup::~BlitGroup(){
  for (unsigned i = 0; i < mBlits.size(); ++i){
    delete mBlits[i];
  }
}

void BlitGroup::render(const Vec2i& pos, const Vec2f& scale, const Vec2i& parentsize, const Color& color, float rotation){
  for (unsigned i = 0; i < mBlits.size(); ++i){
    mBlits[i]->setColor(color);
    mBlits[i]->setRotation(rotation);
    mBlits[i]->render(pos, scale, parentsize);
  }
}

void BlitGroup::setDepth(int depth){
  for (unsigned i = 0; i < mBlits.size(); ++i){
    mBlits[i]->setDepth(depth);
  }
}

void BlitGroup::setBlendAdditive(bool additive){
  for (unsigned i = 0; i < mBlits.size(); ++i){
    mBlits[i]->setBlendAdditive(additive);
  }
}

BlitGroup* BlitGroup::clone(){
  BlitGroup* bltgrp = new BlitGroup();
  for (unsigned i = 0; i < mBlits.size(); ++i){
    BlitObject* bltobj = mBlits[i]->clone();
    bltgrp->mBlits.push_back(bltobj);
  }
  return bltgrp;
}

Animation::Animation(float fps) : mInterval((unsigned)(1000.0f/fps)), mCurrFrame(0), mTimeAccu(0), mHandler(NULL){

}

Animation::Animation(ExtendedFrames& frames, float fps, int depth) : mInterval((unsigned)(1000.0f/fps)), mTimeAccu(0),
mCurrFrame(0), mHandler(NULL){
  for (unsigned k = 0; k < frames.size(); ++k){
    BlitGroup* group = new BlitGroup(frames[k].names, frames[k].offsets, depth);
    mBlits.push_back(group);
  }
}

Animation::Animation(Frames& frames, float fps, Vec2i offset, int depth) : mInterval((unsigned)(1000.0f/fps)), 
mTimeAccu(0), mCurrFrame(0), mHandler(NULL){
  for (unsigned k = 0; k < frames.size(); ++k){
    BlitGroup* group = new BlitGroup(frames[k], offset, depth);
    mBlits.push_back(group);
  }
}

Animation::~Animation(){
  for (unsigned k = 0; k < mBlits.size(); ++k){
    delete mBlits[k];
  }
}

void Animation::render(const Vec2i& pos, const Vec2f& scale, const Vec2i& parentsize, const Color& color, float rotation){
  if (mBlits.size() > mCurrFrame)
    mBlits[mCurrFrame]->render(pos, scale, parentsize, color, rotation);
}

void Animation::setDepth(int depth){
  for (unsigned k = 0; k < mBlits.size(); ++k){
    mBlits[k]->setDepth(depth);
  }
}

void Animation::start(){
  mCurrFrame = 0;
  mTimeAccu = 0;
}

void Animation::update(unsigned interval){
  mTimeAccu += interval;
  while(mTimeAccu >= mInterval){
    mTimeAccu -= mInterval;
    ++mCurrFrame;
    if (mCurrFrame >= mBlits.size()){
      if (mHandler){
        if (mHandler->animationEnded(this))
          mHandler = NULL;
      }
      mCurrFrame = 0;
    }
  }
}

Animation* Animation::clone(){
  Animation* anim = new Animation(1000.0f/mInterval);
  for (unsigned i = 0; i < mBlits.size(); ++i){
    BlitGroup* bltgrp = mBlits[i]->clone();
    anim->mBlits.push_back(bltgrp);
  }
  return anim;
}

void Animation::setBlendAdditive(bool additive){
  for (unsigned k = 0; k < mBlits.size(); ++k){
    mBlits[k]->setBlendAdditive(additive);
  }
}

Object2D::Object2D(int state, const Vec2i& pos, const Vec2i& size, const std::string& name)
: mState(state), mPos(pos), mSize(size), mScript(NULL), mSuspensionScript(NULL), mName(name),
mLightingColor(), mScale(1.0f), mUserScale(1.0f), mRotAngle(0.0f){

}

Object2D::~Object2D(){
  Engine::instance()->remove(this);
  Engine::instance()->getAnimator()->remove(this);
  Engine::instance()->getInterpreter()->remove(this);
  for (unsigned i = 0; i < mAnimations.size(); ++i){
    delete mAnimations[i];
  }
}

void Object2D::render(){
  if (mState <= 0 || (unsigned)mState > mAnimations.size())
    return;
  mAnimations[mState-1]->render(mPos+mScrollOffset, Vec2f(mScale*mUserScale,mScale*mUserScale), mSize, mLightingColor, mRotAngle);
}

Animation* Object2D::getAnimation(){
  if (mState <= 0 || (unsigned)mState > mAnimations.size())
    return NULL;
  return mAnimations[mState-1];
}

bool Object2D::isHit(const Vec2i& point){
  if (mScript == NULL || mState == 0)
    return false;
  //Vec2i scaleoffset;
  //scaleoffset.x = (int)((1.0f-abs(mScale))*(getSize().x-getSize().x*abs(mScale)));
  //scaleoffset.y = (int)(getSize().y-getSize().y*mScale);
  if (point.x >= mPos.x/*+scaleoffset.x*/ && point.x <= mPos.x+/*scaleoffset.x*/+getSize().x){
    if (point.y >= mPos.y/*+scaleoffset.y*/ && point.y <= mPos.y+/*scaleoffset.y*/+getSize().y)
      return true;
  }
  return false;
}

void Object2D::animationEnd(const Vec2i& prev){
  if (mSuspensionScript){
    mSuspensionScript->resume();
    mSuspensionScript = NULL;
  }
}

void Object2D::setSuspensionScript(ExecutionContext* script){
  if (mSuspensionScript != NULL){
    mSuspensionScript->reset(true,true);
  }
  mSuspensionScript = script;
}

void Object2D::save(){
  SaveStateProvider::SaveObject* save = Engine::instance()->getSaver()->getOrAddObject(mName);
  if (save){
    save->position = mPos;
    save->state = mState;
  }
}

int Object2D::getDepth(){
  return mPos.y/Engine::instance()->getWalkGridSize();
}

void Object2D::setDepth(int depth){
  for (unsigned i = 0; i < mAnimations.size(); ++i){
    mAnimations[i]->setDepth(depth);
  }
}

bool Object2D::animationEnded(Animation* anim){
  activateNextState();
  return mNextStates.empty();
}

void Object2D::activateNextState(){
  if (mNextStates.empty())
    return;
  mState = mNextStates.front();
  mNextStates.pop_front();
}

Object2D* Object2D::clone(){
  Object2D* ret = new Object2D(mState, mPos, mSize, mName);
  for (unsigned i = 0; i < mAnimations.size(); ++i){
    Animation* anim = mAnimations[i]->clone();
    ret->addAnimation(anim);
  }
  ret->mRotAngle = mRotAngle;
  return ret;
}

unsigned Object2D::getNumDefinedStates(){
  for (unsigned i = 0; i < mAnimations.size(); ++i){
    if (!mAnimations[i]->exists())
      return i;
  }
  return mAnimations.size();
}

void Object2D::setLighten(bool lighten){
  for (unsigned i = 0; i < mAnimations.size(); ++i){
    mAnimations[i]->setBlendAdditive(lighten);
  }
}

ButtonObject::ButtonObject(const Vec2i& pos, const Vec2i& size, const std::string& text, int id) : Object2D(1, pos, size, "#button"),
BlitObject(Engine::instance()->getSettings()->tsbackground, DEPTH_BUTTON, Vec2i()), mText(text){
  char tmp[16];
  sprintf(tmp, "%i", id);
  mName += tmp;
  mState = Engine::instance()->getInterpreter()->getVariable(mName).getInt();
  if (mState == 0)
    mState = 1;
  Engine::instance()->getInterpreter()->setVariable(mName, 1);
  BaseBlitObject::mPos = pos;
  BaseBlitObject::mSize = size;
  mBackgroundColor = Engine::instance()->getSettings()->tsareacolor;
  mBorderColor = Engine::instance()->getSettings()->tsbordercolor;
  mHighlightColor = Engine::instance()->getSettings()->tsselectioncolor;
  mTextColor = Engine::instance()->getSettings()->tstextcolor;
  if (Engine::instance()->getSettings()->tsstyle == TS_TRANSPARENT){
    mBackgroundColor.a = 127;
    mHighlightColor.a = 127;
  }
  CodeSegment* code = new CodeSegment();
  CBNEEVT* click = new CBNEEVT(EVT_CLICK);
  code->addCode(click);
  code->addCode(new CPUSH(id));
  code->addCode(new CPUSH("#button"));
  code->addCode(new CCALL(ScriptFunctions::setNum, 2));
  click->setOffset(4);
  CBNEEVT* mouse = new CBNEEVT(EVT_MOUSE);
  mScript = new ExecutionContext(code, false, "");
  code->addCode(mouse);
  code->addCode(new CPUSH(2));
  code->addCode(new CPUSH(mName));
  code->addCode(new CCALL(ScriptFunctions::setNum, 2));
  mouse->setOffset(4);
}

ButtonObject::~ButtonObject(){
}

void ButtonObject::setColors(const Color& background, const Color& border, const Color& highlight){
  mBackgroundColor = background;
  mBorderColor = border;
  mHighlightColor = highlight;
  mTextColor = Engine::instance()->getSettings()->tstextcolor;
}

void ButtonObject::render(){
  std::vector<Vec2i> breakinfo;
  breakinfo.push_back(Vec2i(mText.size(), 0)); //fake break
  //Engine::instance()->getFontRenderer()->getTextExtent(mText, 1, breakinfo);
  FontRenderer::String* str = Engine::instance()->getFontRenderer()->render(Object2D::mPos.x, Object2D::mPos.y, mText, DEPTH_UI_FONT, Engine::instance()->getFontID(), breakinfo, mTextColor);
  Engine::instance()->insertToBlit(this);
}

void ButtonObject::blit(){
  if (mTex != 0){
    BlitObject::blit();
    return;
  }
  GL()pushMatrix();
  GL()disable(GL_TEXTURE_2D);
  if (mState == 1)
    GL()color4ub(mBackgroundColor.r, mBackgroundColor.g, mBackgroundColor.b, mBackgroundColor.a);
  else if (mState == 2)
    GL()color4ub(mHighlightColor.r, mHighlightColor.g, mHighlightColor.b, mHighlightColor.a);
  GL()translatef((GLfloat)BaseBlitObject::mPos.x,(GLfloat)BaseBlitObject::mPos.y,0.0f);
  GL()scalef((GLfloat)BaseBlitObject::mSize.x,(GLfloat)BaseBlitObject::mSize.y,1.0f);
  GL()drawArrays(GL_TRIANGLE_STRIP, 0, 4);
  GL()color4ub(mBorderColor.r, mBorderColor.g, mBorderColor.b, mBorderColor.a);
  short indices[] = {
    2, 3, 1, 0
  };
  GL()drawElements(GL_LINE_STRIP, 4, GL_UNSIGNED_SHORT, indices);
  GL()enable(GL_TEXTURE_2D);
  GL()color4ub(255,255,255,255);
  GL()popMatrix();
}

CursorObject::CursorObject(const Vec2i& pos) : Object2D(1, pos, Vec2i(32,32), "xxx"){

}

CursorObject::~CursorObject(){

}

void CursorObject::addAnimation(Animation* anim, int command){
  Object2D::addAnimation(anim);
  mCommands.push_back(command);
}

int CursorObject::getNextCommand(bool& leftClickRequired){
  if (mState == 0)
    return 0;
  ++mState;
  if (mState-1 >= (int)mAnimations.size()-1 || !mAnimations[mState-1]->exists()){
    if (mState == 2){
      mState = 1;
      leftClickRequired = true;
      return mCommands[mState]; //take the next action
    }
    else
      mState = 1;
  }
  leftClickRequired = false;
  return mCommands[mState-1];
}

int CursorObject::getCurrentCommand(){
  if (mState == 0)
    return 0;
  return mCommands[mState-1];
}

void CursorObject::setCommand(int command){
  for (unsigned i = 0; i < mCommands.size(); ++i){
    if (mCommands[i] == command){
      mState = i+1;
      if (mState-1 >= (int)mAnimations.size()-1 || !mAnimations[mState-1]->exists()){
        mState = 1;
      }
      break;
    }
  }
}

RoomObject::RoomObject(int state, const Vec2i& pos, const Vec2i& size, const std::string& name, const Vec2i& depthmap) : 
Object2D(state, pos, size, name), mInventroy(NULL), mDepthMap(depthmap){
  mLighting = new LightingBlitObject(DEPTH_LIGHTING, size);
  mParallaxBackground = NULL;
}

RoomObject::~RoomObject(){
  for (unsigned i = 0; i < mObjects.size(); ++i){
    delete mObjects[i];
  }
  delete mLighting;
  for (std::map<Vec2i,ExecutionContext*>::iterator iter = mWalkmapScripts.begin(); iter != mWalkmapScripts.end(); ++iter){
    Engine::instance()->getInterpreter()->remove(iter->second);
  }
  delete mInventroy;
  delete mParallaxBackground;
}

void RoomObject::render(){
  if (mParallaxBackground)
    mParallaxBackground->render(Vec2i(), Vec2f(mScale*mUserScale,mScale*mUserScale), mSize, mLightingColor, mRotAngle);
  Object2D::render();
  for (int i = mObjects.size()-1; i >= 0; --i){
    mObjects[i]->render();
  }
  CharacterObject* currChar = Engine::instance()->getCharacter("self");
  if (mInventroy && currChar){
    mInventroy->render(currChar->getInventory());
  }
  mLighting->render(Vec2i());
}

void RoomObject::setBackground(std::string bg, int depth){
  if (bg.empty())
    return;
  Frames f;
  f.push_back(bg);
  Animation* anim = new Animation(f, 2.5f, Vec2i(0,0), depth);
  addAnimation(anim);
}

void RoomObject::setParallaxBackground(const std::string& bg, int depth){
  if (bg.empty())
    return;
  Frames f;
  f.push_back(bg);
  mParallaxBackground = new Animation(f, 2.5f, Vec2i(), depth);
}

void RoomObject::addObject(Object2D* obj){
  obj->setScrollOffset(mScrollOffset);
  //obj->setScale(getDepthScale(obj->getPosition()));
  mObjects.push_back(obj);
}

Object2D* RoomObject::getObjectAt(const Vec2i& pos){
  Object2D* curr = NULL;
  int currdepth = -10000;
  for (unsigned i = 0; i < mObjects.size(); ++i){
    if(mObjects[i]->isHit(pos-mScrollOffset)){
      if (mObjects[i]->getDepth() > currdepth){
        curr = mObjects[i];
        currdepth = curr->getDepth();
      }
    }
  }
  if (curr)
    return curr;
  CharacterObject* currChar = Engine::instance()->getCharacter("self");
  if (mInventroy && currChar){
    curr = mInventroy->getObjectAt(pos, currChar->getInventory());
  }
  return curr;
}

Object2D* RoomObject::getObject(const std::string& name){
  for (unsigned i = 0; i < mObjects.size(); ++i){
    if(_stricmp(mObjects[i]->getName().c_str(), name.c_str()) == 0)
      return mObjects[i];
  }
  return NULL;
}

CharacterObject* RoomObject::extractCharacter(const std::string& name){
  for (std::vector<Object2D*>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter){
    if ((*iter)->getType() == Object2D::CHARACTER){
      CharacterObject* ch = static_cast<CharacterObject*>((*iter));
      if (_stricmp(ch->getName().c_str(), name.c_str()) == 0){
        mObjects.erase(iter);
        Engine::instance()->getSaver()->getRoom(mName);
        Engine::instance()->getSaver()->removeCharacter(name);
        return ch;
      }
    }
  }
  return NULL;
}

CharacterObject* RoomObject::findCharacter(const std::string& name){
  for (std::vector<Object2D*>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter){
    if ((*iter)->getType() == Object2D::CHARACTER){
      CharacterObject* ch = static_cast<CharacterObject*>((*iter));
      if (_stricmp(ch->getName().c_str(), name.c_str()) == 0){
        return ch;
      }
    }
  }
  return NULL;
}

bool RoomObject::isWalkable(const Vec2i& pos){
  WMField field = mWalkmap[pos.x][pos.y];
  return field.walkable;
}

void RoomObject::update(unsigned interval){
  for (std::vector<Object2D*>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter){
    Animation* anim = (*iter)->getAnimation();
    if (anim != NULL)
      anim->update(interval);
  }
}

void RoomObject::walkTo(const Vec2i& pos){
  std::map<Vec2i,ExecutionContext*>::iterator iter = mWalkmapScripts.find(pos);
  if (iter == mWalkmapScripts.end())
    return;
  ExecutionContext* scr = iter->second;
  Engine::instance()->getInterpreter()->execute(scr, true);
}

void RoomObject::setScrollOffset(const Vec2i& offset){
  mScrollOffset=offset;
  if (mScrollOffset.x > 0)
    mScrollOffset.x = 0;
  if (mScrollOffset.y > 0)
    mScrollOffset.y = 0;
  if (mScrollOffset.x < Engine::instance()->getResolution().x-mSize.x)
    mScrollOffset.x = Engine::instance()->getResolution().x-mSize.x;
  if (mScrollOffset.y < Engine::instance()->getResolution().y-mSize.y)
    mScrollOffset.y = Engine::instance()->getResolution().y-mSize.y;
  for (std::vector<Object2D*>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter){
    (*iter)->setScrollOffset(mScrollOffset);
  }
}

void RoomObject::save(){
  SaveStateProvider::SaveRoom* save = Engine::instance()->getSaver()->getRoom(mName);
  save->base.position = mPos;
  save->base.state = mState;
  save->base.lighting = mLighting->getColor();
  save->scrolloffset = mScrollOffset;
  for (unsigned i = 0; i < mObjects.size(); ++i){
    mObjects[i]->save();
  }
}

void RoomObject::setPosition(const Vec2i& pos){
  Vec2i move = pos - mPos;
  for (std::vector<Object2D*>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter){
    (*iter)->setPosition((*iter)->getPosition()+move);
    //(*iter)->setScale(getDepthScale((*iter)->getPosition()));
  }
  mPos += move;
  if (mInventroy)
    mInventroy->setPosition(mInventroy->getPosition()+move);
}

bool RoomObject::containsObject(Object2D* object){
  for (std::vector<Object2D*>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter){
    if ((*iter) == object)
      return true;
  }
  return false;
}

void RoomObject::setInventory(InventoryDisplay* disp){
  mInventroy = disp;
  mInventroy->setPosition(mInventroy->getPosition()+mPos);
}

bool RoomObject::isScriptRunning(){
  for (std::vector<Object2D*>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter){
    if ((*iter)->getScript() != NULL && (*iter)->getScript()->isRunning())
      return true;
  }
  if (mScript != NULL && mScript->isRunning())
    return true;
  return false;
}

Vec2i RoomObject::getScriptPosition(ExecutionContext* wmscript){
  for (std::map<Vec2i,ExecutionContext*>::iterator iter = mWalkmapScripts.begin(); iter != mWalkmapScripts.end(); ++iter){
    if (iter->second == wmscript)
      return iter->first;
  }
  return Vec2i(-1,-1);
}

void RoomObject::skipScripts(){
  for (std::vector<Object2D*>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter){
    if ((*iter)->getScript() != NULL)
      (*iter)->getScript()->setSkip();
  }
  if (mScript != NULL && mScript->isRunning())
    mScript->setSkip();
}

float RoomObject::getDepthScale(const Vec2i& pos){
  float factor = (pos.y-mDepthMap.scaleStart)/(float)(mDepthMap.scaleStop-mDepthMap.scaleStart);
  factor = factor < 0 ? 0 : factor;
  factor = factor > 1.0f ? 1.0f : factor;
  float ret = (1-factor)*1+(factor)*mDepthMap.minVal;
  return ret;
}

RoomObject::DepthMap::DepthMap(Vec2i depthmap){
  scaleStart = depthmap.y*Engine::instance()->getWalkGridSize();
  scaleStop = depthmap.x*Engine::instance()->getWalkGridSize();
  setZoomFactor(3);
}

void RoomObject::DepthMap::setZoomFactor(int factor){
  minVal = (float)(1.0f-(scaleStart-scaleStop)/((float)Engine::instance()->getSettings()->resolution.y)*factor*0.3f);
  //minVal = (float)(1.0f-(depthmap.y-depthmap.x)/((float)Engine::instance()->getSettings()->resolution.y/Engine::instance()->getWalkGridSize())*zoomfactor);
}

void RoomObject::setOpacity(unsigned char opacity){
  mLightingColor.a = opacity;
  for (std::vector<Object2D*>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter){
    Color c = (*iter)->getLightingColor();
    c.a = opacity;
    (*iter)->setLightingColor(c);
  }
}

CharacterObject::CharacterObject(int state, Vec2i pos, const std::string& name) 
: Object2D(state, pos, Vec2i(0,0), name), mMirror(false), mTextColor(), 
mFontID(0)
{
  mInventory = new Inventory();
}

CharacterObject::~CharacterObject(){
  Engine::instance()->getFontRenderer()->removeText(this);
  SoundEngine::instance()->removeSpeaker(this);
  delete mInventory;
}

void CharacterObject::setPosition(const Vec2i& pos){
  Vec2i offset = mBasePoints[mState-1];
  Object2D::setPosition(pos-offset);
  RoomObject* room = Engine::instance()->getRoom(mRoom);
  if (room){
    float scale = room->getDepthScale(pos);
    setScale(scale);
  }
}

Vec2i CharacterObject::getPosition(){
  if (mState < 1 || mState >= (int)mAnimations.size())
    return Vec2i();
  return mPos+mBasePoints[mState-1];
}

void CharacterObject::setDepth(int depth){
  for (unsigned i = 0; i < mAnimations.size(); ++i){
    mAnimations[i]->setDepth(depth);
  }
}

void CharacterObject::animationBegin(const Vec2i& next){
  Vec2i dir = next-getPosition();
  setLookDir(dir);
  mState = calculateState(mState, true, isTalking());
}

void CharacterObject::animationWaypoint(const Vec2i& prev, const Vec2i& next){
  int ycoord = getPosition().y;
  if (prev.y-ycoord != 0){
    setDepth(ycoord/Engine::instance()->getWalkGridSize());
  }
  Vec2i dir = next-getPosition();
  setLookDir(dir);
}

void CharacterObject::animationEnd(const Vec2i& prev){
  int ycoord = getPosition().y;
  if (prev.y-ycoord != 0){
    setDepth(ycoord/Engine::instance()->getWalkGridSize());
  }
  if (mDesiredDir != UNSPECIFIED){
    setLookDir(mDesiredDir);
    mDesiredDir = UNSPECIFIED;
  }
  mState = calculateState(mState, false, isTalking());
  Object2D::animationEnd(prev);
}

void CharacterObject::setLookDir(LookDir dir){
  bool walking = isWalking();
  bool talking = isTalking();
  if (dir == FRONT){
    mState = 1;
    mMirror = false;
  }
  else if (dir == BACK){
    mState = 2;
    mMirror = false;
  }
  else if (dir == LEFT){
    mState = 3;
    mMirror = true;
  }
  else if (dir == RIGHT){
    mState = 3;
    mMirror = false;
  }
  mState = calculateState(mState, walking, talking);
}

void CharacterObject::setLookDir(const Vec2i& dir){
  if (abs(dir.x) >= abs(dir.y) && dir.x >= 0){
    setLookDir(RIGHT);
  }
  else if (abs(dir.x) >= abs(dir.y) && dir.x <= 0){
    setLookDir(LEFT);
  }
  else if (abs(dir.x) <= abs(dir.y) && dir.y <= 0){
    setLookDir(BACK);
  }
  else if (abs(dir.x) <= abs(dir.y) && dir.y >= 0){
    setLookDir(FRONT);
  }
}

LookDir CharacterObject::getLookDir(){
  //TODO check if contains bug
  return (LookDir)((mState-1)%3);
}

void CharacterObject::render(){
  if (mState <= 0 || (unsigned)mState > mAnimations.size())
    return;
  Vec2f scale(mScale*mUserScale, mScale*mUserScale);
  if (mMirror)
    scale.x *= -1;
  Vec2i renderPos = mPos+mBasePoints[mState-1]-mBasePoints[mState-1]*mScale*mUserScale;
  mAnimations[mState-1]->render(mScrollOffset+renderPos, scale, mSizes[mState-1], mLightingColor, mRotAngle);
}

Vec2i CharacterObject::getOverheadPos(){
  return mPos+mScrollOffset+Vec2i(mSizes[mState-1].x/2, (int)((1-mScale*mUserScale)*mSizes[mState-1].y));
}

int CharacterObject::calculateState(int currState, bool shouldWalk, bool shouldTalk){
  int stateoffset = (currState-1)%3;
  if (shouldWalk && shouldTalk){
    return stateoffset+10;
  }
  if (shouldWalk && !shouldTalk){
    return stateoffset+4;
  }
  if (!shouldWalk && shouldTalk){
    return stateoffset+7;
  }
  if (!shouldWalk && !shouldTalk){
    return stateoffset+1;
  }
  return stateoffset;
}

bool CharacterObject::isWalking(){
  return (mState >= 4 && mState <= 6) || (mState >= 10 && mState <= 12);
}

bool CharacterObject::isTalking(){
  return (mState >= 7 && mState <= 12);
}

Vec2i CharacterObject::getSize(){
  return mSizes[mState-1]*mScale*mUserScale;
}

void CharacterObject::save(){
  SaveStateProvider::CharSaveObject* save = Engine::instance()->getSaver()->getOrAddCharacter(mName);
  if (save){
    save->base.position = mPos;
    save->base.state = mState;
    save->mirrored = mMirror;
    save->inventory.items.clear();
    if (mInventory){
      mInventory->save(save->inventory);
    }
    save->fontid = mFontID;
    save->scale = mUserScale;
  }
}

bool CharacterObject::isHit(const Vec2i& point){
  if (mScript == NULL || mState == 0)
    return false;
  //Vec2i scaleoffset;
  //scaleoffset.x = (int)((1.0f-abs(mScale))*(getSize().x-getSize().x*abs(mScale)));
  //scaleoffset.y = (int)(getSize().y-getSize().y*mScale);
  Vec2i startPos = mPos+mBasePoints[mState-1]-mBasePoints[mState-1]*mScale*mUserScale;
  if (point.x >= startPos.x/*+scaleoffset.x*/ && point.x <= startPos.x+/*scaleoffset.x*/+getSize().x){
    if (point.y >= startPos.y/*+scaleoffset.y*/ && point.y <= startPos.y+/*scaleoffset.y*/+getSize().y)
      return true;
  }
  return false;
}
