#ifndef CGE_JOINT_H
#define CGE_JOINT_H

#include <ode/ode.h>

namespace CGE{

class Simulator;
class Body;

class Joint{
public:
  Joint(const Simulator& sim);
  ~Joint();
  void attach(const Body* body1, const Body* body2);
protected:
  dJointID mJoint;
};

}

#endif
