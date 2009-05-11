#ifndef COLLISION_BODY_H
#define COLLISION_BODY_H

#include "CollisionObject.h"
#include "Body.h"

namespace CGE{
  class VertexBuffer;
  class IndexBuffer;
}

namespace CGE{

class CollisionSpace;

class CollisionBody : public CollisionObject, public Body{
public:
  CollisionBody(const Simulator& sim);
  virtual ~CollisionBody();
  virtual Type getType();
  virtual void setPosition(const Vec3f& pos) {Body::setPosition(pos);}
  virtual void setOrientation(const Matrix& orientation) {Body::setOrientation(orientation);}
  virtual Vec3f getPosition() {return Body::getPosition();}
  virtual Matrix getOrientation() {return Body::getOrientation();}
  virtual void render(const CGE::Camera& cam);
  virtual void initCylinder(const CollisionSpace& space, float height, float radius, float mass);
  virtual void initBox(const CollisionSpace& space, float width, float height, float depth, float mass);
  virtual void initSphere(const CollisionSpace& space, float radius, float mass);
protected:
  CGE::VertexBuffer* mPhysicsVB;
  CGE::IndexBuffer* mPhysicsInds[3];
};

}

#endif
