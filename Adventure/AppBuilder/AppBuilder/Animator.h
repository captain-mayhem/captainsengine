#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <list>
#include <map>
#include "AdvDoc.h"

class Object2D;
class RoomObject;
class DynamicAnimation;
struct Color;

class Animator{
  struct ObjectAnim{
    //Object2D* object;
    Vec2i startpos;
    std::list<Vec2i> path;
    int speedfactor;
    float normalization;
    float factor;
  };
public:
  Animator();
  ~Animator();
  void add(Object2D* obj, const std::list<Vec2i>& targetpath, int speedfactor);
  void remove(Object2D* obj);
  void update(unsigned interval);
  void clear();
  void add(DynamicAnimation* anim);
  void add(Object2D* obj, const Color& targetcolor);
  void add(RoomObject* obj, Vec2i scrollpos);
protected:
  std::map<Object2D*, ObjectAnim> mObjects;
  std::list<DynamicAnimation*> mAnimations;
};

#endif
