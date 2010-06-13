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
    RoomObject* ro = Engine::instance()->getContainingRoom(iter->first);
    if (ro)
      iter->first->setScale(ro->getDepthScale(reachedpos));
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

void Animator::add(Object2D* obj, const Color& targetcolor){
  //TODO Lighting animation
  obj->setLightingColor(targetcolor);
  obj->getScript()->resume();
}
