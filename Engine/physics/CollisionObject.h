#ifndef CGE_COLLISION_OBJECT_H
#define CGE_COLLISION_OBJECT_H

#include <ode/ode.h>
#include "../math/vector.h"
#include "../math/matrix.h"

namespace Graphics{
  class Camera;
}

namespace CGE{

class CollisionObject{
  friend class CollisionSpace;
public:
  CollisionObject();
  virtual ~CollisionObject();
  virtual void setPosition(const Vec3f& pos);
  virtual void setOrientation(const Matrix& orientation);
  virtual Vec3f getPosition();
  virtual Matrix getOrientation();
  virtual void render(const Graphics::Camera& cam)=0;
protected:
  dGeomID mGeom;
};

}

#endif
