#ifndef CGE_COLLISION_PLANE
#define CGE_COLLISION_PLANE

#include "CollisionObject.h"

namespace CGE{

class CollisionPlane : public CollisionObject{
public:
  CollisionPlane(const Vec3f& normal, float distance);
  virtual ~CollisionPlane();
  virtual void render(const CGE::Camera& cam);
};

}

#endif
