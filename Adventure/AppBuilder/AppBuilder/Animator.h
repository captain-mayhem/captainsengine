#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <list>
#include <map>
#include "AdvDoc.h"

class Object2D;

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
  void update(unsigned interval);
  void clear();
protected:
  std::map<Object2D*, ObjectAnim> mObjects;
};

#endif
