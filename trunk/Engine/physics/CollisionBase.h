#ifndef CGE_COLLISIONBASE_H
#define CGE_COLLISIONBASE_H

namespace CGE{

class CollisionBase{
public:
  enum Type{
    Undefined,
    Space,
    Cylinder,
    Box,
    Sphere,
    Heightfield,
    Plane,
  };
  CollisionBase() {}
  virtual ~CollisionBase() {}
  virtual Type getType() {return Undefined;}
};

}

#endif
