#ifndef COLLISION_BODY_H
#define COLLISION_BODY_H

#include <memory>

#include "CollisionObject.h"
#include "Body.h"
#include "../renderer/forms.h"

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
  virtual void render(Gfx::DrawContext& ctx, const CGE::Camera& cam);
  virtual void initCylinder(const CollisionSpace& space, float height, float radius, float mass);
  virtual void initBox(const CollisionSpace& space, float width, float height, float depth, float mass);
  virtual void initSphere(const CollisionSpace& space, float radius, float mass);
protected:
  //! the shape on the engine's render device, drawn by render
  std::unique_ptr<Forms::Shape> mShape;
};

}

#endif
