#ifndef CGE_COLLISION_OBJECT_H
#define CGE_COLLISION_OBJECT_H

#include <ode/ode.h>
#include "../math/vector.h"
#include "../math/matrix.h"
#include "CollisionBase.h"

namespace CGE{
  class Camera;
  namespace Gfx{
  class DrawContext;
  }
}

namespace CGE{

class CollisionObject : public CollisionBase{
  friend class CollisionSpace;
  friend class Simulator;
public:
  CollisionObject();
  virtual ~CollisionObject();
  virtual void setPosition(const Vec3f& pos);
  virtual void setOrientation(const Matrix& orientation);
  virtual Vec3f getPosition();
  virtual Matrix getOrientation();
  virtual void render(Gfx::DrawContext& ctx, const CGE::Camera& cam)=0;
protected:
  dGeomID mGeom;
};

}

#endif
