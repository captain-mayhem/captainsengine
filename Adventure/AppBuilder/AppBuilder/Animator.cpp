#include "Animator.h"
#include "EngineObjects.h"

Animator::Animator(){

}

Animator::~Animator(){

}

void Animator::add(Object2D* obj, const std::queue<Vec2i>& targetpath, int speedfactor){
  ObjectAnim anim;
  anim.object = obj;
  anim.startpos = obj->getPosition();
  anim.path = targetpath;
  anim.speedfactor = speedfactor;
  anim.normalization = (targetpath.front()-anim.startpos).length();
  anim.factor = 0;
  if (anim.normalization != 0.0f){
    obj->animationBegin();
    //if (obj->getAnimation()){
    //  obj->getAnimation()->start();
    //}
    mObjects.push_back(anim);
  }
}

void Animator::update(unsigned interval){
  for (std::list<ObjectAnim>::iterator iter = mObjects.begin(); iter != mObjects.end(); ++iter){
    iter->factor += interval*iter->speedfactor/20.f;
    if (iter->factor > iter->normalization)
      iter->factor = iter->normalization;
    Vec2i oldpos = iter->startpos;
    Vec2i newpos = iter->path.front();
    Vec2i dir = newpos-oldpos;
    //float factor = iter->factor/iter->normalization;
    float newx = oldpos.x+dir.x/iter->normalization*iter->factor;
    float newy = oldpos.y+dir.y/iter->normalization*iter->factor;
    Vec2i reachedpos(newx,newy);
    iter->object->setPosition(reachedpos);
    if (iter->object->getAnimation()){
      iter->object->getAnimation()->update(interval);
    }
    if (iter->factor >= iter->normalization){
      //goal reached
      iter->path.pop();
      if (iter->path.empty()){
        iter->object->animationEnd();
        iter = mObjects.erase(iter);
        if (iter == mObjects.end())
          break;
      }
      else{
        iter->startpos = reachedpos;
        iter->normalization = (iter->path.front()-iter->startpos).length();
        iter->factor = 0;
      }
    }
  }
}

void Animator::clear(){
  mObjects.clear();
}
