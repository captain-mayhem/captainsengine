#ifndef CGE_COLLISION_HEIGHTFIELD_H
#define CGE_COLLISION_HEIGHTFIELD_H

#include <ode/ode.h>
#include "../math/vector.h"
#include "../math/matrix.h"
#include "CollisionObject.h"

namespace CGE{
  class Camera;
}

namespace CGE{

class CollisionHeightfield : public CollisionObject{
  friend class CollisionSpace;
public:
  CollisionHeightfield();
  virtual ~CollisionHeightfield();
  virtual void render(const CGE::Camera& cam);
  void init(int16 width, int16 depth, float widthscale, float depthscale, uint8* heightdata, float heightscale);
};

}

#endif
