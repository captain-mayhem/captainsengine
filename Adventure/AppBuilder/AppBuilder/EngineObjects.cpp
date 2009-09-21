#include "EngineObjects.h"
#include <wx/image.h>
#include "Engine.h"
#include "SaveStateProvider.h"

BaseBlitObject::BaseBlitObject(int depth, const Vec2i& size) : 
mPos(), mSize(size), mDepth(depth){

}

BaseBlitObject::~BaseBlitObject(){

}

BlitObject::BlitObject(std::string texture, int depth, Vec2i offset) : 
BaseBlitObject(depth, Vec2i()), mOffset(offset){
  wxImage image = Engine::instance()->getImage(texture);
  mTex = Engine::instance()->genTexture(image, mSize, mScale);
  mDeleteTex = true;
}

BlitObject::BlitObject(GLuint texture, const Vec2i& size, const Vec2f& scale, int depth, const Vec2i& offset):
BaseBlitObject(depth, size), mOffset(offset), mScale(scale), mTex(texture)
{
  mMirrorX = false;
  mDeleteTex = false;
}

BlitObject::~BlitObject(){
  if (mDeleteTex)
    glDeleteTextures(1, &mTex);
}

bool BlitObject::operator<(const BlitObject& obj){
  return mDepth < obj.mDepth;
}

void BlitObject::render(Vec2i pos, bool mirrorx){
  mPos.x = mOffset.x+pos.x;
  mPos.y = mOffset.y+pos.y;
  mMirrorX = mirrorx;
  Engine::instance()->insertToBlit(this);
}

void BlitObject::blit(){
  glPushMatrix();
  //if (mMirrorX)
  //  glTranslatef(mPos.x+mOffset.x,mPos.y,0.0f);
  //else
    glTranslatef(mPos.x,mPos.y,0.0f);
  glScalef(mSize.x,mSize.y,1.0f);
  if (mMirrorX)
    glScalef(-1.,1.,1.);
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  glScalef(mScale.x, mScale.y, 1.0f);
  glMatrixMode(GL_MODELVIEW);
  glBindTexture(GL_TEXTURE_2D, mTex);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glPopMatrix();
}

LightingBlitObject::LightingBlitObject(int depth, const Vec2i& size) : BaseBlitObject(depth, size){
}

LightingBlitObject::~LightingBlitObject(){

}

void LightingBlitObject::render(const Vec2i& pos){
  mPos = pos;
  Engine::instance()->insertToBlit(this);
}

void LightingBlitObject::blit(){
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glPushMatrix();
  glTranslatef(mPos.x,mPos.y,0.0f);
  glScalef(mSize.x,mSize.y,1.0f);
  glColor4ub(mColor.r, mColor.g, mColor.b, mColor.a);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glColor4ub(255, 255, 255, 255);
  glDisable(GL_BLEND);
  glPopMatrix();
  glEnable(GL_TEXTURE_2D);
}

ScrollBlitObject::ScrollBlitObject(int depth) : BaseBlitObject(depth, Vec2i(0,0)){
}

ScrollBlitObject::~ScrollBlitObject(){
}

void ScrollBlitObject::blit(){
  if (mDepth < 0){
    glPushMatrix();
    glTranslatef(mPos.x, mPos.y, 0);
  }
  else
    glPopMatrix();
}
  
void ScrollBlitObject::render(const Vec2i& pos){
  mPos = pos;
}

///

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

void BlitGroup::render(Vec2i pos, bool mirrorx){
  for (unsigned i = 0; i < mBlits.size(); ++i){
    mBlits[i]->render(pos, mirrorx);
  }
}

void BlitGroup::setDepth(int depth){
  for (unsigned i = 0; i < mBlits.size(); ++i){
    mBlits[i]->setDepth(depth);
  }
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

void Animation::render(Vec2i pos, bool mirrorx){
  //if (mCurrFrame != 0){
  //  mBlits[mCurrFrame]->render(pos, mirrorx);
  //}
  if (mBlits.size() > mCurrFrame)
    mBlits[mCurrFrame]->render(pos, mirrorx);
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
        mHandler->animationEnded(this);
        mHandler = NULL;
      }
      mCurrFrame = 0;
    }
  }
}

Object2D::Object2D(int state, const Vec2i& pos, const Vec2i& size, const std::string& name)
: mState(state), mPos(pos), mSize(size), mScript(NULL), mSuspensionScript(NULL), mName(name){

}

Object2D::~Object2D(){
  Engine::instance()->getAnimator()->remove(this);
  Engine::instance()->getInterpreter()->remove(mScript);
  for (unsigned i = 0; i < mAnimations.size(); ++i){
    delete mAnimations[i];
  }
}

void Object2D::render(){
  if (mState <= 0 || (unsigned)mState > mAnimations.size())
    return;
  mAnimations[mState-1]->render(mPos+mScrollOffset, false);
}

Animation* Object2D::getAnimation(){
  if (mState <= 0 || (unsigned)mState > mAnimations.size())
    return NULL;
  return mAnimations[mState-1];
}

bool Object2D::isHit(const Vec2i& point){
  if (mScript == NULL)
    return false;
  if (point.x >= mPos.x && point.x <= mPos.x+getSize().x){
    if (point.y >= mPos.y && point.y <= mPos.y+getSize().y)
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
    mSuspensionScript->reset(true);
  }
  mSuspensionScript = script;
}

void Object2D::save(){
  SaveStateProvider::SaveObject* save = Engine::instance()->getSaver()->getObject(mName);
  if (save){
    save->position = mPos;
    save->state = mState;
  }
}

int Object2D::getDepth(){
  return mPos.y/Engine::instance()->getWalkGridSize();
}

CursorObject::CursorObject(const Vec2i& pos) : Object2D(1, pos, Vec2i(32,32), "xxx"){

}

CursorObject::~CursorObject(){

}

void CursorObject::addAnimation(Animation* anim, int command){
  Object2D::addAnimation(anim);
  mCommands.push_back(command);
}

int CursorObject::getNextCommand(){
  if (mState == 0)
    return 0;
  ++mState;
  if (mState-1 >= (int)mAnimations.size()-1 || !mAnimations[mState-1]->exists()){
    mState = 1;
  }
  return mCommands[mState-1];
}

RoomObject::RoomObject(const Vec2i& size, const std::string& name) : Object2D(1, Vec2i(0,0), size, name){
  mLighting = new LightingBlitObject(1000, size);
}

RoomObject::~RoomObject(){
  for (unsigned i = 0; i < mObjects.size(); ++i){
    delete mObjects[i];
  }
  delete mLighting;
  for (std::map<Vec2i,ExecutionContext*>::iterator iter = mWalkmapScripts.begin(); iter != mWalkmapScripts.end(); ++iter){
    Engine::instance()->getInterpreter()->remove(iter->second);
  }
}

void RoomObject::render(){
  Object2D::render();
  for (unsigned i = 0; i < mObjects.size(); ++i){
    mObjects[i]->render();
  }
  mLighting->render(Vec2i());
}

void RoomObject::setBackground(std::string bg){
  Frames f;
  f.push_back(bg);
  Animation* anim = new Animation(f, 2.5f, Vec2i(0,0), -1);
  addAnimation(anim);
}

void RoomObject::addObject(Object2D* obj){
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
  return curr;
}

Object2D* RoomObject::getObject(const std::string& name){
  for (unsigned i = 0; i < mObjects.size(); ++i){
    if(mObjects[i]->getName() == name)
      return mObjects[i];
  }
  return NULL;
}

CharacterObject* RoomObject::extractCharacter(const std::string& name){
  for (std::vector<Object2D*>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter){
    if ((*iter)->getType() == Object2D::CHARACTER){
      CharacterObject* ch = static_cast<CharacterObject*>((*iter));
      if (ch->getName() == name){
        mObjects.erase(iter);
        Engine::instance()->getSaver()->getRoom(mName);
        Engine::instance()->getSaver()->removeObject(name);
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
      if (ch->getName() == name){
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
  save->lighting = mLighting->getColor();
  for (unsigned i = 0; i < mObjects.size(); ++i){
    mObjects[i]->save();
  }
}

CharacterObject::CharacterObject(int state, Vec2i pos, const std::string& name) 
: Object2D(state, pos, Vec2i(0,0), name), mMirror(false), mTextColor(), 
mFontID(0), mNextState(-1)
{

}

CharacterObject::~CharacterObject(){
  Engine::instance()->getFontRenderer()->removeText(this);
}

void CharacterObject::setPosition(const Vec2i& pos){
  Vec2i offset = mBasePoints[mState-1];
  Object2D::setPosition(pos-offset);
}

Vec2i CharacterObject::getPosition(){
  if (mState < 1 || mState >= (int)mAnimations.size())
    return Vec2i();
  return mPos+mBasePoints[mState-1];
}
/*
Vec2i CharacterObject::calcPosition(const Vec2i& p){
  return mBasePoints[mState]+p;
}*/

void CharacterObject::setDepth(int depth){
  for (unsigned i = 0; i < mAnimations.size(); ++i){
    mAnimations[i]->setDepth(depth);
  }
}

void CharacterObject::animationBegin(const Vec2i& next){
  Vec2i dir = next-getPosition();
  if (abs(dir.x) > abs(dir.y) && dir.x > 0){
    setLookDir(RIGHT);
  }
  else if (abs(dir.x) > abs(dir.y) && dir.x < 0){
    setLookDir(LEFT);
  }
  else if (abs(dir.x) < abs(dir.y) && dir.y < 0){
    setLookDir(BACK);
  }
  else if (abs(dir.x) < abs(dir.y) && dir.y > 0){
    setLookDir(FRONT);
  }
  mState = calculateState(mState, true, isTalking());
}

void CharacterObject::animationWaypoint(const Vec2i& prev, const Vec2i& next){
  int ycoord = getPosition().y;
  if (prev.y-ycoord != 0){
    setDepth(ycoord/Engine::instance()->getWalkGridSize());
  }
  Vec2i dir = next-getPosition();
  if (abs(dir.x) > abs(dir.y) && dir.x > 0){
    setLookDir(RIGHT);
  }
  else if (abs(dir.x) > abs(dir.y) && dir.x < 0){
    setLookDir(LEFT);
  }
  else if (abs(dir.x) < abs(dir.y) && dir.y < 0){
    setLookDir(BACK);
  }
  else if (abs(dir.x) < abs(dir.y) && dir.y > 0){
    setLookDir(FRONT);
  }
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

void CharacterObject::animationEnded(Animation* anim){
  mState = mNextState;
  mNextState = -1;
}

void CharacterObject::setLookDir(LookDir dir){
  int stateoffset = (mState-1)/3;
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
  mState += 3*stateoffset;
}

LookDir CharacterObject::getLookDir(){
  return (LookDir)((mState-1)%3);
}

void CharacterObject::render(){
  if (mState <= 0 || (unsigned)mState > mAnimations.size())
    return;
  if (mMirror)
    mAnimations[mState-1]->render(mScrollOffset+mPos+Vec2i(mBasePoints[mState-1].x,0), mMirror);
  else
    mAnimations[mState-1]->render(mScrollOffset+mPos, mMirror);
}

Vec2i CharacterObject::getOverheadPos(){
  return mPos+mScrollOffset+Vec2i(mSizes[mState-1].x/2, 0);
}

void CharacterObject::activateNextState(){
  if (mNextState != -1)
    mState = mNextState;
  mNextState = -1;
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
  return mSizes[mState-1];
}
