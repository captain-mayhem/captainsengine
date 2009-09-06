#include "Animator.h"

#include <queue>

#include "EngineObjects.h"

Animator::Animator(){

}

Animator::~Animator(){

}

void Animator::add(Object2D* obj, const std::list<Vec2i>& targetpath, int speedfactor){
  std::map<Object2D*,ObjectAnim>::iterator iter = mObjects.find(obj);
  if (iter != mObjects.end()){
    obj->animationEnd(targetpath.front());
    mObjects.erase(iter);
  }
  if (targetpath.empty())
    return;
  ObjectAnim anim;
  //anim.object = obj;
  anim.startpos = obj->getPosition();
  anim.path = targetpath;
  anim.speedfactor = speedfactor;
  anim.normalization = (targetpath.front()-anim.startpos).length();
  anim.factor = 0;
  if (anim.normalization != 0.0f){
    obj->animationBegin(targetpath.front());
    //if (obj->getAnimation()){
    //  obj->getAnimation()->start();
    //}
    mObjects[obj] = anim;
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
    Vec2i reachedpos(newx,newy);
    iter->first->setPosition(reachedpos);
    if (iter->second.factor >= iter->second.normalization){
      //goal reached
      iter->second.path.pop_front();
      if (iter->second.path.empty()){
        iter->first->animationEnd(iter->second.startpos);
        iter = mObjects.erase(iter);
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
}
