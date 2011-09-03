#include "Animator.h"

#include <queue>

#include "EngineObjects.h"
#include "Engine.h"

Animator::Animator(){

}

Animator::~Animator(){

}

void Animator::add(Object2D* obj, const std::list<Vec2i>& targetpath, int speedfactor){
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
}

void Animator::update(unsigned interval){
  for (std::map<Object2D*, ObjectAnim>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter){
    iter->second.factor += interval*iter->second.speedfactor/20.f;
    if (iter->second.factor > iter->second.normalization)
      iter->second.factor = iter->second.normalization;
    Vec2i oldpos = iter->second.startpos;
    Vec2i newpos = iter->second.path.front();
    Vec2i dir = newpos-oldpos;
    //float factor = iter->factor/iter->normalization;
    float newx = oldpos.x+dir.x/iter->second.normalization*iter->second.factor;
    float newy = oldpos.y+dir.y/iter->second.normalization*iter->second.factor;
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
private:
  int mDuration;
  int mCurrentTime;
  Color mSrc;
  Color mTarget;
  Object2D* mObject;
};

void Animator::add(Object2D* obj, const Color& targetcolor){
  ColorAnimation* ca = new ColorAnimation(obj->getLightingColor(), targetcolor, obj);
  add(ca);
}

class ScrollAnimation : public DynamicAnimation{
public:
  ScrollAnimation(RoomObject* room, Vec2i target, float speed) : mRoom(room), 
    mDir(target-mRoom->getScrollOffset()), mCurrPos(mRoom->getScrollOffset()), 
    mTarget(target), mSpeed(speed){
    mDir.normalize();
  }
  virtual bool update(unsigned interval){
    if ((mCurrPos-mTarget).length() < 0.5){
      mRoom->setScrollOffset(mTarget);
      return false;
    }
    mCurrPos += mDir*interval*mSpeed;
    mRoom->setScrollOffset(Vec2i((int)mCurrPos.x, (int)mCurrPos.y));
    return true;
  }
private:
  RoomObject* mRoom;
  Vec2f mDir;
  Vec2f mCurrPos;
  Vec2i mTarget;
  float mSpeed;
};

void Animator::add(RoomObject* obj, Vec2i scrollpos){
  ScrollAnimation* sa = new ScrollAnimation(obj, scrollpos, 0.1f);
  add(sa);
}
