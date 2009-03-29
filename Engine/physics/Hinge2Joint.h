#ifndef CGE_HINGE2_JOINT
#define CGE_HINGE2_JOINT

#include "Joint.h"
#include "../math/vector.h"

namespace CGE{

class Hinge2Joint : public Joint{
public:
  Hinge2Joint(const Simulator& sim);
  ~Hinge2Joint();
  void setAnchor(const Vec3f& point);
  void setAxis1(const Vec3f& axis);
  void setAxis2(const Vec3f& axis);
  void setVelocityAxis2(float velocity);
  void setMaxForceAxis2(float force);
};

}

#endif
