#include "Animator.h"

#include <queue>

#include "EngineObjects.h"
#include "Engine.h"
#include <system/allocation.h>

TR_CHANNEL(ADV_Animator);

using namespace adv;

Animator::Animator(){

}

Animator::~Animator(){

}

void Animator::add(Object2D* obj, const std::list<Vec2i>& targetpath, float speedfactor){
  std::map<Object2D*,ObjectAnim>::iterator iter = mObjects.find(obj);
  if (iter != mObjects.end()){
    if (!targetpath.empty())
      obj->animationEnd(targetpath.front());
    mObjects.erase(iter);
  }
  if (targetpath.empty()){
    obj->animationEnd(obj->getPosition());
    return;
  }
  ObjectAnim anim;
  //anim.object = obj;
  anim.startpos = obj->getPosition();
  anim.path = targetpath;
  anim.speedfactor = speedfactor;
  while (!anim.path.empty()){
    anim.normalization = (anim.path.front()-anim.startpos).length();
    if (anim.normalization == 0.0f){
      anim.path.pop_front();
    }
    else{
      break;
    }
  }
  anim.factor = 0;
  if (anim.normalization != 0.0f){
    obj->animationBegin(anim.path.front());
    mObjects[obj] = anim;
  }
  else{
    obj->animationEnd(anim.startpos);
  }
}

void Animator::remove(Object2D* obj){
  std::map<Object2D*,ObjectAnim>::iterator iter = mObjects.find(obj);
  if (iter != mObjects.end()){
    obj->animationEnd(obj->getPosition());
    mObjects.erase(iter);
  }
  for (std::list<DynamicAnimation*>::iterator iter = mAnimations.begin(); iter != mAnimations.end(); ++iter){
    if ((*iter)->getTarget() == obj){
      (*iter)->finish();
      delete *iter;
      iter = mAnimations.erase(iter);
      if (iter == mAnimations.end())
        break;
    }
  }
}

void Animator::remove(RoomObject* room){
  std::map<RoomObject*,RoomAnim>::iterator iter = mRooms.find(room);
  if (iter != mRooms.end()){
    mRooms.erase(iter);
  }
  for (std::list<DynamicAnimation*>::iterator iter = mAnimations.begin(); iter != mAnimations.end(); ++iter){
    if ((*iter)->getTarget() == room){
      (*iter)->finish();
      delete *iter;
      iter = mAnimations.erase(iter);
      if (iter == mAnimations.end())
        break;
    }
  }
}

bool Animator::isPointOnLine(Vec2f from, Vec2f to, Vec2f pt, double epsilon)
{
  double segmentLengthSqr = (to.x - from.x) * (to.x - from.x) + (to.y - from.y) * (to.y - from.y);
  double r = ((pt.x - from.x) * (to.x - from.x) + (pt.y - from.y) * (to.y - from.y)) / segmentLengthSqr;
  if(r<0 || r>1) return false;
  double sl = ((from.y - pt.y) * (to.x - from.x) - (from.x - pt.x) * (to.y - from.y)) / sqrt(segmentLengthSqr);
  return -epsilon <= sl && sl <= epsilon;
}


void Animator::update(unsigned interval){
  TR_USE(ADV_Animator);
  if (interval == 0)
    return;
  for (std::map<Object2D*, ObjectAnim>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter){
    iter->second.factor += interval*iter->second.speedfactor/20.f;
    if (iter->second.factor > iter->second.normalization)
      iter->second.factor = iter->second.normalization;
    Vec2i oldpos = iter->second.startpos;
    Vec2i newpos = iter->second.path.front();
    Vec2i dir = newpos-oldpos;
    float factor = iter->second.factor;
    if (iter->first->getType() == Object2D::CHARACTER){
      CharacterObject* obj = (CharacterObject*)iter->first; //slow down for smaller characters
      factor *= (((obj->getScaleFactor()-1.0f)*0.5f)+1.0f);
    }
    float newx = oldpos.x+dir.x/iter->second.normalization*factor;
    float newy = oldpos.y+dir.y/iter->second.normalization*factor;
    Vec2i reachedpos((int)newx,(int)newy);
    iter->first->setPosition(reachedpos);
    //RoomObject* ro = Engine::instance()->getContainingRoom(iter->first);
    //if (ro)
    //  iter->first->setScale(ro->getDepthScale(reachedpos));
    if (iter->second.factor >= iter->second.normalization){
      //goal reached
      iter->second.path.pop_front();
      if (iter->second.path.empty()){
        iter->first->animationEnd(iter->second.startpos);
        mObjects.erase(iter++);
        if (iter == mObjects.end())
          break;
      }
      else{
        Vec2i tmp = iter->second.startpos;
        iter->second.startpos = reachedpos;
        while(iter->second.path.front() == iter->second.startpos)
          iter->second.path.pop_front();
        iter->second.normalization = (iter->second.path.front()-iter->second.startpos).length();
        iter->second.factor = 0;
        iter->first->animationWaypoint(tmp, iter->second.path.front());
      }
    }
  }

  for (std::list<DynamicAnimation*>::iterator iter = mAnimations.begin(); iter != mAnimations.end(); ++iter){
    bool cont = (*iter)->update(interval);
    if (!cont){
      delete (*iter);
      iter = mAnimations.erase(iter);
      if (iter == mAnimations.end())
        break;
    }
  }

  int pos = 0;
  for (std::map<RoomObject*, RoomAnim>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    if ((iter->second.currPos-iter->second.target).length() <= 0.1){
      iter->first->setScrollOffset(iter->second.target);
      mRooms.erase(iter);
      //reposition manually as erase does only return valid iterator on Windows
      iter = mRooms.begin();
      for (int i = 0; i < pos; ++i)
        ++iter;
      if (iter == mRooms.end())
        break;
    }
    Vec2f oldpos = iter->second.currPos;
    iter->second.currPos += iter->second.dir*(float)interval*iter->second.speed;
    TR_DEBUG("Scroll offset: move from %f %f to %f %f (interval %i)", oldpos.x, oldpos.y, iter->second.currPos.x, iter->second.currPos.y, interval);
    if (!isPointOnLine(oldpos, iter->second.target, iter->second.currPos, 0.5f))
      iter->second.currPos = iter->second.target;
    iter->first->setScrollOffset(Vec2i((int)iter->second.currPos.x, (int)iter->second.currPos.y));
    ++pos;
  }

  if (mCamera.active){
    Vec2i dims = Engine::instance()->getSettings()->resolution;
    mCamera.currPos = mCamera.currPos+mCamera.dir*(float)interval;
    glViewport((int)mCamera.currPos.x, (int)mCamera.currPos.y, dims.x, dims.y);
    glScissor((int)mCamera.currPos.x, (int)mCamera.currPos.y, dims.x, dims.y);
    if (mCamera.dir.x > 0 && mCamera.currPos.x > mCamera.target.x
      || mCamera.dir.x < 0 && mCamera.currPos.x < mCamera.target.x
      || mCamera.dir.y > 0 && mCamera.currPos.y > mCamera.target.y
      || mCamera.dir.y < 0 && mCamera.currPos.y < mCamera.target.y){
        glViewport(0, 0, dims.x, dims.y);
        glDisable(GL_SCISSOR_TEST);
        mCamera.active = false;
    }
  }
}

void Animator::clear(){
  mObjects.clear();
  for (std::list<DynamicAnimation*>::iterator iter = mAnimations.begin(); iter != mAnimations.end(); ++iter){
    delete *iter;
  }
  mAnimations.clear();
}

void Animator::add(DynamicAnimation* anim){
  mAnimations.push_back(anim);
}

class ColorAnimation : public DynamicAnimation{
public:
  ColorAnimation(const Color& src, const Color& target, Object2D* object) : mDuration(300), 
    mCurrentTime(0), mSrc(src), mTarget(target), mObject(object){
  }
  virtual bool update(unsigned interval){
    float t = (mDuration-mCurrentTime)/(float)mDuration;
    if (t < 0)
      t = 0;
    if (t > 1.0f)
      t = 1.0f;
    Color col;
    col.r = (unsigned char)(t*mSrc.r + (1-t)*mTarget.r);
    col.g = (unsigned char)(t*mSrc.g + (1-t)*mTarget.g);
    col.b = (unsigned char)(t*mSrc.b + (1-t)*mTarget.b);
    col.a = (unsigned char)(t*mSrc.a + (1-t)*mTarget.a);
    mObject->setLightingColor(col);
    if (mCurrentTime >= mDuration)
      return false;
    mCurrentTime += interval;
    return true;
  }
  virtual void finish(){
    mObject->setLightingColor(mTarget);
  }
  virtual Object2D* getTarget() {return mObject;}
  virtual Type getType() {return COLOR;}
  const Color& getTargetColor() {return mTarget;}
private:
  int mDuration;
  int mCurrentTime;
  Color mSrc;
  Color mTarget;
  Object2D* mObject;
};

void Animator::add(Object2D* obj, const Color& targetcolor){
  //remove previous animation
  for (std::list<DynamicAnimation*>::iterator iter = mAnimations.begin(); iter != mAnimations.end(); ++iter){
    if ((*iter)->getType() == DynamicAnimation::COLOR && (*iter)->getTarget() == obj){
      (*iter)->finish();
      delete *iter;
      mAnimations.erase(iter);
      break;
    }
  }
  ColorAnimation* ca = new ColorAnimation(obj->getLightingColor(), targetcolor, obj);
  add(ca);
}

class OpacityAnimation : public DynamicAnimation{
public:
  OpacityAnimation(int from, int to, int duration, RoomObject* object, bool reset) : mDuration(duration), 
    mCurrentTime(0), mSrc(from), mTarget(to), mRoom(object), mReset(reset){
  }
  virtual bool update(unsigned interval){
    float t = (mDuration-mCurrentTime)/(float)mDuration;
    if (t < 0)
      t = 0;
    if (t > 1.0f)
      t = 1.0f;
    unsigned char opacity = (unsigned char)(t*mSrc + (1-t)*mTarget);
    mRoom->setOpacity(opacity);
    if (mCurrentTime >= mDuration){
      if (mReset)
        mRoom->setFadeout(0);
      return false;
    }
    mCurrentTime += interval;
    return true;
  }
  virtual void finish(){
    mRoom->setOpacity(mTarget);
    if (mReset)
      mRoom->setFadeout(0);
  }
  virtual Object2D* getTarget() {return mRoom;}
  virtual Type getType() {return OPACITY;}
private:
  int mDuration;
  int mCurrentTime;
  int mSrc;
  int mTarget;
  RoomObject* mRoom;
  bool mReset;
};

void Animator::add(RoomObject* obj, int fade_duration, bool fadein){
  OpacityAnimation* oa = new OpacityAnimation(fadein ? 0 : 255, fadein ? 255 : 0, fade_duration, obj, !fadein);
  add(oa);
}

void Animator::add(RoomObject* obj, Vec2i scrollpos, float speed){
  std::map<RoomObject*,RoomAnim>::iterator iter = mRooms.find(obj);
  Vec2f currPos;
  if (iter != mRooms.end()) //get exact current position of a running animation
    currPos = iter->second.currPos;
  else
    currPos = obj->getScrollOffset();
  RoomAnim anim;
  anim.dir = scrollpos-currPos;
  anim.dir.normalize();
  anim.currPos = currPos;
  anim.target = scrollpos;
  anim.speed = speed;
  mRooms[obj] = anim;
}

class ScaleAnimation : public DynamicAnimation{
public:
  ScaleAnimation(const float src, const float target, CharacterObject* object, bool scaleUserScale) : mDuration(1000), 
    mCurrentTime(0), mSrc(src), mTarget(target), mObject(object), mScaleUserScale(scaleUserScale){
      mDuration = (int)(abs(mSrc-mTarget)*mDuration);
  }
  virtual bool update(unsigned interval){
    float t = (mDuration-mCurrentTime)/(float)mDuration;
    if (t < 0)
      t = 0;
    if (t > 1.0f)
      t = 1.0f;
    if (!mScaleUserScale)
      mTarget = mObject->getScale();
    //else
    //  mTarget = mObject->getUserScale();
    float scale = t*mSrc + (1-t)*mTarget;
    if (!mScaleUserScale)
      mObject->setFrozenScale(scale);
    else
      mObject->setUserScale(scale);
    if (mCurrentTime >= mDuration){
      //TODO: scripting (charzoom): nozooming has to remain on
      if (!mScaleUserScale)
        mObject->setNoZooming(false, true);
      return false;
    }
    mCurrentTime += interval;
    return true;
  }
  virtual void finish(){
    if (!mScaleUserScale){
      mObject->setFrozenScale(mTarget);
      mObject->setNoZooming(false, true);
    }
    else
      mObject->setUserScale(mTarget);
  }
  virtual Object2D* getTarget() {return mObject;}
  virtual Type getType() {return SCALE;}
  float getTargetScale() {return mTarget;}
private:
  int mDuration;
  int mCurrentTime;
  float mSrc;
  float mTarget;
  CharacterObject* mObject;
  bool mScaleUserScale;
};

void Animator::add(CharacterObject* obj, float sourcescale, float targetscale, bool isUserScale){
  //remove previous animation
  for (std::list<DynamicAnimation*>::iterator iter = mAnimations.begin(); iter != mAnimations.end(); ++iter){
    if ((*iter)->getType() == DynamicAnimation::SCALE && (*iter)->getTarget() == obj){
      ScaleAnimation* sa = (ScaleAnimation*)*iter;
      delete *iter;
      mAnimations.erase(iter);
      break;
    }
  }
  ScaleAnimation* sa = new ScaleAnimation(sourcescale, targetscale, obj, isUserScale);
  add(sa);
}

Vec2i Animator::getTargetPoisition(Object2D* obj){
  std::map<Object2D*,ObjectAnim>::iterator iter = mObjects.find(obj);
  if (iter == mObjects.end())
    return obj->getPosition();
  return iter->second.path.back();
}

void Animator::moveCameraViewport(LookDir dir){
  Vec2i dims = Engine::instance()->getSettings()->resolution;
  if (dir == LEFT){
    mCamera.currPos = Vec2f((float)-dims.x, 0);
    mCamera.dir = Vec2f(1, 0);
    mCamera.target = Vec2f(0, 0);
  }
  else if (dir == RIGHT){
    mCamera.currPos = Vec2f((float)dims.x, 0);
    mCamera.dir = Vec2f(-1, 0);
    mCamera.target = Vec2f(0, 0);
  }
  else if (dir == FRONT){
    mCamera.currPos = Vec2f(0, (float)-dims.y);
    mCamera.dir = Vec2f(0, 1);
    mCamera.target = Vec2f(0, 0);
  }
  else if (dir == BACK){
    mCamera.currPos = Vec2f(0, (float)dims.y);
    mCamera.dir = Vec2f(0, -1);
    mCamera.target = Vec2f(0, 0);
  }
  mCamera.active = true;
  glEnable(GL_SCISSOR_TEST);
  glScissor((int)mCamera.currPos.x, (int)mCamera.currPos.y, dims.x, dims.y);
}
