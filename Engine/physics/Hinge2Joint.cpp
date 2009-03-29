
#include "Hinge2Joint.h"

#include "Simulator.h"

using namespace CGE;

Hinge2Joint::Hinge2Joint(const CGE::Simulator &sim) : Joint(sim){
  mJoint = dJointCreateHinge2(sim.mWorld, 0);
}

Hinge2Joint::~Hinge2Joint(){

}

void Hinge2Joint::setAnchor(const Vec3f& anchor){
  dJointSetHinge2Anchor(mJoint, anchor.x, anchor.y, anchor.z);
}

void Hinge2Joint::setAxis1(const CGE::Vec3f &axis){
  dJointSetHinge2Axis1(mJoint, axis.x, axis.y, axis.z);
}

void Hinge2Joint::setAxis2(const CGE::Vec3f &axis){
  dJointSetHinge2Axis2(mJoint, axis.x, axis.y, axis.z);
}

void Hinge2Joint::setVelocityAxis2(float velocity){
  dJointSetHinge2Param(mJoint, dParamVel2, velocity);
}

void Hinge2Joint::setMaxForceAxis2(float force){
  dJointSetHinge2Param(mJoint, dParamFMax2, force);
}
