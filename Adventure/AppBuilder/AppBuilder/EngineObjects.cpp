#include "EngineObjects.h"
#include <wx/image.h>
#include "Engine.h"

BlitObject::BlitObject(std::string texture, int depth, Vec2i offset) : 
mDepth(depth), mOffset(offset){
  wxImage image = Engine::instance()->getImage(texture);
  mTex = Engine::instance()->genTexture(image, mSize, mScale);
  mDeleteTex = true;
}

BlitObject::BlitObject(GLuint texture, const Vec2i& size, const Vec2f& scale, int depth, const Vec2i& offset):
mOffset(offset), mSize(size), mScale(scale), mDepth(depth), mTex(texture)
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

Animation::Animation(float fps) : mInterval(1000/fps), mCurrFrame(0){

}

Animation::Animation(ExtendedFrames& frames, float fps, int depth) : mInterval(1000/fps), mCurrFrame(0){
  for (unsigned k = 0; k < frames.size(); ++k){
    BlitGroup* group = new BlitGroup(frames[k].names, frames[k].offsets, depth);
    mBlits.push_back(group);
  }
}

Animation::Animation(Frames& frames, float fps, Vec2i offset, int depth) : mInterval(1000/fps), mCurrFrame(0){
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
    if (mCurrFrame >= mBlits.size())
      mCurrFrame = 0;
  }
}

Object2D::Object2D(int state, const Vec2i& pos, const Vec2i& size) : mState(state), 
mPos(pos), mSize(size), mScript(NULL){

}

Object2D::~Object2D(){
  delete mScript;
  for (unsigned i = 0; i < mAnimations.size(); ++i){
    delete mAnimations[i];
  }
}

void Object2D::render(){
  if (mState <= 0 || (unsigned)mState > mAnimations.size())
    return;
  mAnimations[mState-1]->render(mPos, false);
}

Animation* Object2D::getAnimation(){
  if (mState <= 0 || (unsigned)mState > mAnimations.size())
    return NULL;
  return mAnimations[mState-1];
}

bool Object2D::isHit(const Vec2i& point){
  if (mScript == NULL)
    return false;
  if (point.x >= mPos.x && point.x <= mPos.x+mSize.x){
    if (point.y >= mPos.y && point.y <= mPos.y+mSize.y)
      return true;
  }
  return false;
}

RoomObject::RoomObject(const Vec2i& size) : Object2D(1, Vec2i(0,0), size){

}

RoomObject::~RoomObject(){
  for (unsigned i = 0; i < mObjects.size(); ++i){
    delete mObjects[i];
  }
}

void RoomObject::render(){
  Object2D::render();
  for (unsigned i = 0; i < mObjects.size(); ++i){
    mObjects[i]->render();
  }
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
  for (unsigned i = 0; i < mObjects.size(); ++i){
    if(mObjects[i]->isHit(pos))
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

CharacterObject::CharacterObject(int state, Vec2i pos, const std::string& name) 
: Object2D(state, pos, Vec2i(0,0)), mName(name), mMirror(false)
{

}

CharacterObject::~CharacterObject(){

}

void CharacterObject::setPosition(const Vec2i& pos){
  Vec2i offset = mBasePoints[mState];
  Object2D::setPosition(pos-offset);
}

Vec2i CharacterObject::getPosition(){
  return mPos+mBasePoints[mState];
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
    mState = 3;
    mMirror = false;
  }
  else if (abs(dir.x) > abs(dir.y) && dir.x < 0){
    mState = 3;
    mMirror = true;
  }
  else if (abs(dir.x) < abs(dir.y) && dir.y < 0){
    mState = 2;
    mMirror = false;
  }
  else if (abs(dir.x) < abs(dir.y) && dir.y > 0){
    mState = 1;
    mMirror = false;
  }
  mState += 3;
}

void CharacterObject::animationWaypoint(const Vec2i& prev, const Vec2i& next){
  int ycoord = getPosition().y;
  if (prev.y-ycoord != 0){
    setDepth(ycoord/Engine::instance()->getWalkGridSize());
  }
  Vec2i dir = next-getPosition();
  if (abs(dir.x) > abs(dir.y) && dir.x > 0){
    mState = 3;
    mMirror = false;
  }
  else if (abs(dir.x) > abs(dir.y) && dir.x < 0){
    mState = 3;
    mMirror = true;
  }
  else if (abs(dir.x) < abs(dir.y) && dir.y < 0){
    mState = 2;
    mMirror = false;
  }
  else if (abs(dir.x) < abs(dir.y) && dir.y > 0){
    mState = 1;
    mMirror = false;
  }
  mState += 3;
}

void CharacterObject::animationEnd(const Vec2i& prev){
  int ycoord = getPosition().y;
  if (prev.y-ycoord != 0){
    setDepth(ycoord/Engine::instance()->getWalkGridSize());
  }
  mState -= 3;
}

void CharacterObject::render(){
  if (mState <= 0 || (unsigned)mState > mAnimations.size())
    return;
  if (mMirror)
    mAnimations[mState-1]->render(mPos+Vec2i(mBasePoints[mState-1].x,0), mMirror);
  else
    mAnimations[mState-1]->render(mPos, mMirror);
}
