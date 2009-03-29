#ifndef COLLISION_SPACE_H
#define COLLISION_SPACE_H

#include <ode/ode.h>

#include "../math/vector.h"

namespace CGE{

class CollisionObject;

class CollisionSpace{
  friend class Simulator;
  friend class CollisionBody;
public:
  enum Type{
    Hash,
    Simple,
    QuadTree,
  };
  CollisionSpace(CollisionSpace* parent, Type t, const Vec3f& center = Vec3f(), const Vec3f& extents = Vec3f(1,1,1), const int depth=16);
  ~CollisionSpace();
  void add(CollisionObject* object);
protected:
  dSpaceID mSpace;
};

}

#endif