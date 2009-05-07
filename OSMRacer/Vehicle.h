
#include "math/vector.h"
#include <physics/CollisionBody.h>
#include <physics/CollisionSpace.h>
#include "GameObject.h"

namespace CGE{
  class Camera;
}

namespace CGE{
  class Hinge2Joint;
}

class Vehicle : public CGE::CollisionSpace, public GameObject{
public:
  Vehicle(const CGE::Simulator& sim);
  ~Vehicle();
  virtual CGE::Vec3f getPosition();
  virtual CGE::Vec3f getDirection();
  void setPosition(const CGE::Vec3f& pos);
  void render(const CGE::Camera& cam);
  void simulate(float acceleration, float steering);
protected:
  CGE::CollisionBody* mAutobody;
  CGE::CollisionBody* mWheels[4];
  CGE::Hinge2Joint* mWheelHinges[4];
};
