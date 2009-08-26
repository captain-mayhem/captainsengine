#include "EngineObjects.h"
#include <wx/image.h>
#include "Engine.h"

BlitObject::BlitObject(std::string texture, int depth, Vec2i offset) : 
mDepth(depth), mOffset(offset){
  genTexture(texture);
}

BlitObject::~BlitObject(){
  glDeleteTextures(1, &mTex);
}

//TODO move into utility
unsigned roundToPowerOf2(unsigned x){
  --x;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  return x + 1;
}

void BlitObject::genTexture(const std::string& name){
  std::string filename = Engine::instance()->resolveFilename(IMAGE, name);
  wxImage image(filename);
  mSize.x = image.GetWidth();
  mSize.y = image.GetHeight();
  Vec2i pow2(roundToPowerOf2(mSize.x), roundToPowerOf2(mSize.y));
  mScale.x = ((float)mSize.x)/pow2.x;
  mScale.y = ((float)mSize.y)/pow2.y;
  unsigned totalsize = mSize.x*mSize.y;
  unsigned char* rgb = image.GetData();
  unsigned char* alpha = image.GetAlpha();
  GLubyte* buffer = new GLubyte[totalsize*4];
  for (unsigned i = 0; i < totalsize; ++i){
    buffer[4*i] = rgb[3*i];
    buffer[4*i+1] = rgb[3*i+1];
    buffer[4*i+2] = rgb[3*i+2];
    if (alpha)
      buffer[4*i+3] = alpha[i];
    else{
      if (rgb[3*i] == 0xFF && rgb[3*i+1] == 0x0 && rgb[3*i+2] == 0xFF){
        buffer[4*i+3] = 0;
      }
      else{
        buffer[4*i+3] = 255;
      }
    }
  }
  glGenTextures(1,&mTex);
  glBindTexture(GL_TEXTURE_2D, mTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pow2.x, pow2.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mSize.x, mSize.y, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  delete buffer;
}

bool BlitObject::operator<(const BlitObject& obj){
  return mDepth < obj.mDepth;
}

void BlitObject::render(Vec2i pos){
  mPos.x = mOffset.x+pos.x;
  mPos.y = mOffset.y+pos.y;
  Engine::instance()->insertToBlit(this);
}

void BlitObject::blit(){
  glPushMatrix();
  glTranslatef(mPos.x,mPos.y,0.0f);
  glScalef(mSize.x,mSize.y,1.0f);
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

void BlitGroup::render(Vec2i pos){
  for (unsigned i = 0; i < mBlits.size(); ++i){
    mBlits[i]->render(pos);
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

void Animation::render(Vec2i pos){
  if (mCurrFrame != 0){
    mBlits[mCurrFrame]->render(pos);
  }
  if (mBlits.size() > mCurrFrame)
    mBlits[mCurrFrame]->render(pos);
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

Object2D::Object2D(int state, Vec2i pos) : mState(state), mPos(pos){

}

Object2D::~Object2D(){
  for (unsigned i = 0; i < mAnimations.size(); ++i){
    delete mAnimations[i];
  }
}

void Object2D::render(){
  if (mState <= 0 || (unsigned)mState > mAnimations.size())
    return;
  mAnimations[mState-1]->render(mPos);
}

Animation* Object2D::getAnimation(){
  if (mState <= 0 || (unsigned)mState > mAnimations.size())
    return NULL;
  return mAnimations[mState-1];
}

RoomObject::RoomObject() : Object2D(1, Vec2i(0,0)){

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
: Object2D(state, pos), mName(name), mMirror(true)
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
  mState += 3;
}

void CharacterObject::animationWaypoint(const Vec2i& prev, const Vec2i& next){
  int ycoord = getPosition().y;
  if (prev.y-ycoord != 0){
    setDepth(ycoord/Engine::instance()->getWalkGridSize());
  }
}

void CharacterObject::animationEnd(const Vec2i& prev){
  int ycoord = getPosition().y;
  if (prev.y-ycoord != 0){
    setDepth(ycoord/Engine::instance()->getWalkGridSize());
  }
  mState -= 3;
}

void CharacterObject::render(){
  if (mMirror){
    glPushMatrix();
    glScalef(-1.0,1.0,1.0);
  }
  Object2D::render();
  if (mMirror){
    glPopMatrix();
  }
}
