
#include "../math/vector.h"
#include "../math/matrix.h"

namespace CGE{

class Body{
public:
  enum Type{
    Cylinder
  };
  Body();
  ~Body();
  void initCylinder(float height, float radius, float mass);
  void initBox(float with, float height, float depth, float mass);
  void initSphere(float radius, float mass);
protected:
  //float mMass;
  //CGE::Matrix mInertiaTensor;
  //CGE::Vec3f mPosition;
  //CGE::Vec3f mVelocity;
};

}