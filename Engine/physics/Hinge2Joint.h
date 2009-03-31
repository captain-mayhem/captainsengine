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
  void setVelocityAxis1(float velocity);
  void setVelocityAxis2(float velocity);
  void setMaxForceAxis1(float force);
  void setMaxForceAxis2(float force);
  void setLowStop(float angle);
  void setHighStop(float angle);
  void setFudgeFactor(float factor);
  void setSuspensionERP(float factor);
  void setSuspensionCFM(float factor);
  float getAngleAxis1();
};

}

#endif
