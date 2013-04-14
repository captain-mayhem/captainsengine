#include "EngineObjects.h"
#include "Engine.h"
#include "SaveStateProvider.h"
#include "Inventory.h"
#include "ScriptFunc.h"
#include "Sound.h"

using namespace adv;

TR_CHANNEL_LVL(ADV_Character, TRACE_INFO);
TR_CHANNEL(ADV_Room);

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

void BlitGroup::setBlendMode(BlitObject::BlendMode mode){
  for (unsigned i = 0; i < mBlits.size(); ++i){
    mBlits[i]->setBlendMode(mode);
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

void Animation::setBlendMode(BlitObject::BlendMode mode){
  for (unsigned k = 0; k < mBlits.size(); ++k){
    mBlits[k]->setBlendMode(mode);
  }
}

Object2D::Object2D(int state, const Vec2i& pos, const Vec2i& size, const std::string& name)
: mState(state), mPos(pos), mSize(size), mScript(NULL), mSuspensionScript(NULL), mName(name),
mLightingColor(), mScale(1.0f), mUserScale(1.0f), mRotAngle(0.0f), mDepth(0){

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
  return getAnimation(mState);
}

Animation* Object2D::getAnimation(int state){
  if (state <= 0 || (unsigned)state > mAnimations.size())
    return NULL;
  return mAnimations[state-1];
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
    mSuspensionScript->unref();
    mSuspensionScript = NULL;
  }
}

void Object2D::setSuspensionScript(ExecutionContext* script){
  if (mSuspensionScript != NULL){
    mSuspensionScript->reset(true,true);
    mSuspensionScript->unref();
  }
  script->ref();
  mSuspensionScript = script;
}

void Object2D::save(){
  SaveStateProvider::SaveObject* save = Engine::instance()->getSaver()->getOrAddObject(mName);
  if (save){
    save->position = mPos;
    save->state = mState;
    save->lighting = mLightingColor;
    save->name = mName;
  }
}

int Object2D::getDepth(){
  //return mPos.y/Engine::instance()->getWalkGridSize();
  return mDepth;
}

void Object2D::setDepth(int depth){
  mDepth = depth;
  for (unsigned i = 0; i < mAnimations.size(); ++i){
    mAnimations[i]->setDepth(depth);
  }
}

bool Object2D::animationEnded(Animation* anim){
  activateNextState();
  return mNextStates.empty();
}

int Object2D::removeLastNextState(){ 
  int ret;
  if (!mNextStates.empty()){
    ret = mNextStates.back();
    mNextStates.pop_back();
  }
  else
    ret = getState();
  return ret;
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
    mAnimations[i]->setBlendMode(lighten ? BlitObject::BLEND_ADDITIVE : BlitObject::BLEND_ALPHA);
  }
}

void Object2D::update(unsigned interval){
  Animation* anim = getAnimation();
  if (anim != NULL)
    anim->update(interval);
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
  code->addCode(new CCALL(ScriptFunctions::setNum, "setnum", 2));
  click->setOffset(4);
  CBNEEVT* mouse = new CBNEEVT(EVT_MOUSE);
  mScript = new ExecutionContext(code, false, "");
  code->addCode(mouse);
  code->addCode(new CPUSH(2));
  code->addCode(new CPUSH(mName));
  code->addCode(new CCALL(ScriptFunctions::setNum, "setnum", 2));
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
  Color textcol = mTextColor;
  if (mTex != 0){
    if (mState == 1)
      textcol *= mBackgroundColor;
    else if (mState == 2)
      textcol *= mHighlightColor;
  }
  FontRenderer::String* str = Engine::instance()->getFontRenderer()->render(Object2D::mPos.x, Object2D::mPos.y, mText, DEPTH_UI_FONT, Engine::instance()->getFontID(), breakinfo, textcol);
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
  Engine::instance()->getAnimator()->remove(this);
  for (unsigned i = 0; i < mMirrors.size(); ++i){
    delete mMirrors[i];
  }
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
    if (mObjects[i]->getType() != CHARACTER)
      mObjects[i]->render();
  }
  for (int i = mObjects.size()-1; i >= 0; --i){
    if (mObjects[i]->getType() == CHARACTER)
      mObjects[i]->render();
  }
  CharacterObject* currChar = Engine::instance()->getCharacter("self");
  if (mInventroy && currChar && Engine::instance()->getMainRoom() != this){
    mInventroy->render(currChar->getInventory());
  }
  mLighting->render(Vec2i());
  for (unsigned i = 0; i < mMirrors.size(); ++i){
    mMirrors[i]->update(0);
    mMirrors[i]->render(Vec2i(), Vec2f(1.0f,1.0f), Vec2i());
  }
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
  obj->setPosition(obj->getPosition()+mPos);
  obj->setScrollOffset(mScrollOffset);
  //obj->setScale(getDepthScale(obj->getPosition()));
  mObjects.push_back(obj);
}

Object2D* RoomObject::getObjectAt(const Vec2i& pos){
  Object2D* curr = NULL;
  int currdepth = -10000;
  for (unsigned i = 0; i < mObjects.size(); ++i){
    if(mObjects[i]->isHit(pos-mScrollOffset)){
      if ((mObjects[i]->getType() != Object2D::CHARACTER && mObjects[i]->getDepth() >= currdepth) ||
        (mObjects[i]->getType() == Object2D::CHARACTER && mObjects[i]->getDepth() > currdepth)){
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
    if (mObjects[i]->getType() != Object2D::OBJECT) //find only 'real' objects
      continue;
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
  if (pos.x < 0 || pos.y < 0){
    return false;
  }
  WMField field = mWalkmap[pos.x][pos.y];
  return field.walkable;
}

void RoomObject::update(unsigned interval){
  for (std::vector<Object2D*>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter){
    (*iter)->update(interval);
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
  if (Engine::instance()->getRoom("") == this){
    CharacterObject* chr = Engine::instance()->getCharacter("self");
    if (chr)
      chr->setScrollOffset(mScrollOffset);
  }
}

void RoomObject::save(){
  SaveStateProvider::SaveRoom* save = Engine::instance()->getSaver()->getRoom(mName);
  if (!save)
    return;
  save->base.position = mPos;
  save->base.state = mState;
  save->base.lighting = mLightingColor;
  save->overlaylighting = mLighting->getColor();
  save->scrolloffset = mScrollOffset;
  save->walkmap = mModifiedWalkmap;
  for (unsigned i = 0; i < mObjects.size(); ++i){
    mObjects[i]->save();
  }
}

void RoomObject::setPosition(const Vec2i& pos){
  if (pos.x < -1000 || pos.x > 1000){
    TR_USE(ADV_Room);
    TR_BREAK("Invalid room position %i/%i", pos.x, pos.y);
  }
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

void RoomObject::finishScripts(bool execute){
  for (std::vector<Object2D*>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter){
    if ((*iter)->getScript() != NULL){
      (*iter)->getScript()->finish();
      if (execute && (*iter)->getScript()->isRunning()){
        //(*iter)->getScript()->resume();
        Engine::instance()->getInterpreter()->executeImmediately((*iter)->getScript());
      }
    }
  }
  if (mScript != NULL/* && mScript->isRunning()*/){
    //mScript->resetEvent(EVT_LOOP1);
    //mScript->resetEvent(EVT_LOOP2);
    mScript->finish();
    if (execute && mScript->isRunning()){
      //mScript->resume();
      Engine::instance()->getInterpreter()->executeImmediately(mScript);
    }
  }
}

float RoomObject::getDepthScale(const Vec2i& pos){
  float factor = (pos.y-mDepthMap.scaleStart)/(float)(mDepthMap.scaleStop-mDepthMap.scaleStart);
  factor = factor < 0 ? 0 : factor;
  factor = factor > 1.0f ? 1.0f : factor;
  float ret = (1-factor)*1+(factor)*mDepthMap.minVal;
  return ret;
}

float RoomObject::getDepthScale(const Vec2i& pos, int depthStart, int depthEnd, int zoomfactor){
  float minVal = (float)(1.0f-(depthStart-depthEnd)/((float)Engine::instance()->getSettings()->resolution.y)*zoomfactor*0.3f);

  float factor = (pos.y-depthStart)/(float)(depthEnd-depthStart);
  factor = factor < 0 ? 0 : factor;
  factor = factor > 1.0f ? 1.0f : factor;
  float ret = (1-factor)*1+(factor)*minVal;
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

bool RoomObject::hitsBarriers(const ParticleEngine::Particle& particle){
  for (unsigned i = 0; i < mBarriers.size(); ++i){
    if (mBarriers[i].isHit(particle, mScrollOffset))
      return true;
  }
  return false;
}

void RoomObject::modifyWalkmap(const Vec2i& pos, bool walkable){
  mWalkmap[pos.x-1][pos.y-1].walkable = walkable;
  mModifiedWalkmap[pos] = walkable;
}

void RoomObject::setDepth(int depth){
  mDepth = depth;
}

CharacterObject::CharacterObject(int state, Vec2i pos, const std::string& name) 
: Object2D(state, pos, Vec2i(0,0), name), mMirror(false), mTextColor(), 
mFontID(0), mLinkObject(NULL), mNoZooming(false), mIdleTime(0), mSpawnPos(-1,-1),
mWalkSound(NULL)
{
  mInventory = new Inventory();
  mIdleTimeout = (int(rand()/(float)RAND_MAX*10)+10)*1000;
}

CharacterObject::~CharacterObject(){
  Engine::instance()->getFontRenderer()->removeText(this);
  SoundEngine::instance()->removeSpeaker(this);
  if (mWalkSound)
    SoundEngine::instance()->removeSoundPlayer(mWalkSound);
  delete mInventory;
}

void CharacterObject::setPosition(const Vec2i& pos){
  setPosition(pos, false);
}

void CharacterObject::setPosition(const Vec2i& pos, bool isSpawnPos){
  Vec2i offset;
  if (mState != 0)
    offset = mBasePoints[mState-1];
  Object2D::setPosition(pos-offset);
  if (isSpawnPos)
    mSpawnPos = getPosition();
  RoomObject* room = Engine::instance()->getRoom(mRoom);
  if (room){
    float scale = room->getDepthScale(pos);
    setScale(scale);
  }
  /*else{
    Room* rm = Engine::instance()->getData()->getRoom(mRoom);
    float scale = RoomObject::getDepthScale(pos, rm->depthmap.x, rm->depthmap.y, rm->zoom);
    setScale(scale);
  }*/
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
  setState(calculateState(mState, true, isTalking()));
  if (mWalkSound)
    mWalkSound->play(true);
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
  setState(calculateState(mState, false, isTalking()));
  Object2D::animationEnd(prev);
  if (mWalkSound){
    mWalkSound->stop();
    mWalkSound->seek(0);
  }
}

int CharacterObject::calculateState(LookDir dir, bool& mirror){
  int state = 0;
  if (dir == FRONT){
    state = 1;
    mirror = false;
  }
  else if (dir == BACK){
    state = 2;
    mirror = false;
  }
  else if (dir == LEFT){
    state = 3;
    mirror = true;
  }
  else if (dir == RIGHT){
    state = 3;
    mirror = false;
  }
  return state;
}

void CharacterObject::setLookDir(LookDir dir){
  bool walking = isWalking();
  bool talking = isTalking();
  mState = calculateState(dir, mMirror);
  setState(calculateState(mState, walking, talking));
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

void CharacterObject::render(bool mirrorY){
  if (mState <= 0 || (unsigned)mState > mAnimations.size())
    return;
  Vec2f scale(getScaleFactor(), getScaleFactor());
  if (mMirror)
    scale.x *= -1;
  if (mirrorY)
    scale.y *= -1;
  Vec2i renderPos = renderPos = mPos+mBasePoints[mState-1]-mBasePoints[mState-1]*getScaleFactor();
  if (mirrorY)
    renderPos.y += (int)(mBasePoints[mState-1].y*getScaleFactor()-2);
  mAnimations[mState-1]->render(mScrollOffset+renderPos, scale, mSizes[mState-1], mLightingColor, mRotAngle);
}

Vec2i CharacterObject::getOverheadPos(){
  return mPos+mScrollOffset+Vec2i(mSizes[mState-1].x/2, (int)((1-getScaleFactor())*mSizes[mState-1].y));
}

int CharacterObject::calculateState(int currState, bool shouldWalk, bool shouldTalk, bool mirror){
  int stateoffset = 0;
  if (currState <= 12) //take offset only from 'normal' states
    stateoffset = (currState-1)%3;
  else
    return currState;
  //TODO handle real left anims and special states like pickup
  if (mirror){
    //swap looking front and back
    if (stateoffset == 0)
      stateoffset = 1;
    else if (stateoffset == 1)
      stateoffset = 0;
  }
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
    if (isWalking() || isTalking()) //do not save walking or talking states as they are temorary only
      save->base.state = calculateState(mState, false, false, false);
    else if (mState > 12)
      save->base.state = 1; //no special states
    else
      save->base.state = mState;
    save->base.name = mName;
    save->mirrored = mMirror;
    save->inventory.items.clear();
    if (mInventory){
      mInventory->save(save->inventory);
    }
    save->fontid = mFontID;
    save->scale = mUserScale;
    save->nozooming = mNoZooming;
    if (mWalkSound == NULL)
      save->walksound = "";
    else
      save->walksound = mWalkSound->getName();
  }
}

bool CharacterObject::isHit(const Vec2i& point){
  if (mScript == NULL || mState == 0)
    return false;
  //Vec2i scaleoffset;
  //scaleoffset.x = (int)((1.0f-abs(mScale))*(getSize().x-getSize().x*abs(mScale)));
  //scaleoffset.y = (int)(getSize().y-getSize().y*mScale);
  Vec2i startPos = mPos+mBasePoints[mState-1]-mBasePoints[mState-1]*getScaleFactor();
  if (point.x >= startPos.x/*+scaleoffset.x*/ && point.x <= startPos.x+/*scaleoffset.x*/+getSize().x){
    if (point.y >= startPos.y/*+scaleoffset.y*/ && point.y <= startPos.y+/*scaleoffset.y*/+getSize().y)
      return true;
  }
  return false;
}

void CharacterObject::setState(int state){
  TR_USE(ADV_Character);
  mState = state;
  //mNextStates.clear();
  //fallback to lower states when they not exist (walk,talk  back => walk back)
  if (!getAnimation()->exists()){
    if (mState > 3)
      mState = calculateState(mState, isWalking(), false);
  }
  mIdleTime = 0; //reset idle timer
  TR_DEBUG("state %i", mState);
}

void CharacterObject::update(unsigned interval){
  if (mLinkObject != NULL){
    Vec2i pos = mLinkObject->getPosition()+mLinkObject->getSize()/2;
    setPosition(pos-mLinkOffset);
  }
  else{
    Object2D::update(interval);
    mIdleTime += interval;
    //trigger idle animation
    if (mIdleTime >= mIdleTimeout){
      int oldstate = getState();
      float rnd = rand()/(float)RAND_MAX;
      int nextbored = (int)(rnd+0.5f);
      if (!mAnimations[13-1+nextbored]->exists()){
        nextbored = 1 - nextbored;
      }
      if (mAnimations[13-1+nextbored]->exists()){
        setState(13+nextbored);
        getAnimation()->registerAnimationEndHandler(this);
        addNextState(oldstate);
      }
      mIdleTime = 0;
      mIdleTimeout = (int(rand()/(float)RAND_MAX*10)+10)*1000;
    }
    if (isWalking() || isTalking() || Engine::instance()->getInterpreter()->isBlockingScriptRunning()) //also not idle when taking or walking and no state change occurs
      mIdleTime = 0;
  }
}

float CharacterObject::getScaleFactor(){
  if (mNoZooming)
    return mFrozenScale*mUserScale;
  return mScale*mUserScale;
}

void CharacterObject::setNoZooming(bool nozooming, bool force){
  if (nozooming){
    mNoZooming = nozooming;
    mFrozenScale = mScale;
  }
  else{
    if (force)
      mNoZooming = nozooming;
    else
      Engine::instance()->getAnimator()->add(this, mFrozenScale, mScale, false);
  }
}

void CharacterObject::setLinkObject(Object2D* link){
  mLinkObject = link;
  if (link){
    mLinkOffset = mLinkObject->getPosition()+mLinkObject->getSize()/2-getPosition();
  }
}

int CharacterObject::getDepth(){
  return getPosition().y/Engine::instance()->getWalkGridSize();
}

bool CharacterObject::isSpawnPos(){
  if (getPosition()/Engine::instance()->getWalkGridSize() == mSpawnPos/Engine::instance()->getWalkGridSize())
    return true;
  mSpawnPos = Vec2i(-1,-1);
  return false;
}

void CharacterObject::setWalkSound(SoundPlayer* plyr){
  if (mWalkSound != NULL)
    SoundEngine::instance()->removeSoundPlayer(mWalkSound);
  mWalkSound = plyr;
}

void CharacterObject::abortClick(){
  if (mSuspensionScript != NULL){
    if (mSuspensionScript->getEvents().front() == EVT_CLICK){
      mSuspensionScript->getEvents().pop_front();
      Object2D::animationEnd(Vec2i());
    }
  }
  Engine::instance()->getAnimator()->remove(this);
}
