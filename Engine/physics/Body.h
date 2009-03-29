#ifndef CGE_BODY_H
#define CGE_BODY_H

#include "../math/vector.h"
#include "../math/matrix.h"

#include <ode/ode.h>

namespace CGE{

class Simulator;

class Body{
  friend class CollisionBody;
  friend class Joint;
public:
  Body(const Simulator& simulator);
  virtual ~Body();
  virtual void initCylinder(float height, float radius, float mass);
  virtual void initBox(float width, float height, float depth, float mass);
  virtual void initSphere(float radius, float mass);
  virtual void setPosition(const Vec3f& pos);
  virtual void setOrientation(const Matrix& orientation);
  virtual Vec3f getPosition();
  virtual Matrix getOrientation();
protected:
  dBodyID mBody;
  //float mMass;
  //CGE::Matrix mInertiaTensor;
  //CGE::Vec3f mPosition;
  //CGE::Vec3f mVelocity;
};

}

#endif
