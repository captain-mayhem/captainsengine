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

void Animator::remove(RoomObject* room){
  std::map<RoomObject*,RoomAnim>::iterator iter = mRooms.find(room);
  if (iter != mRooms.end()){
    mRooms.erase(iter);
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

  int pos = 0;
  for (std::map<RoomObject* const, RoomAnim>::iterator iter = mRooms.begin(); iter != mRooms.end(); ++iter){
    if ((iter->second.currPos-iter->second.target).length() < 0.5){
      iter->first->setScrollOffset(iter->second.target);
      mRooms.erase(iter);
      //reposition manually as erase does only return valid iterator on Windows
      iter = mRooms.begin();
      for (int i = 0; i < pos; ++i)
        ++iter;
      if (iter == mRooms.end())
        break;
    }
    iter->second.currPos += iter->second.dir*(float)interval*iter->second.speed;
    iter->first->setScrollOffset(Vec2i((int)iter->second.currPos.x, (int)iter->second.currPos.y));
    ++pos;
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

void Animator::add(RoomObject* obj, Vec2i scrollpos, float speed){
  RoomAnim anim;
  anim.dir = scrollpos-obj->getScrollOffset();
  anim.dir.normalize();
  anim.currPos = obj->getScrollOffset();
  anim.target = scrollpos;
  anim.speed = speed;
  mRooms[obj] = anim;
}
