
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

void Hinge2Joint::setVelocityAxis1(float velocity){
  dJointSetHinge2Param(mJoint, dParamVel, velocity);
}

void Hinge2Joint::setVelocityAxis2(float velocity){
  dJointSetHinge2Param(mJoint, dParamVel2, velocity);
}

void Hinge2Joint::setMaxForceAxis1(float force){
  dJointSetHinge2Param(mJoint, dParamFMax, force);
}

void Hinge2Joint::setMaxForceAxis2(float force){
  dJointSetHinge2Param(mJoint, dParamFMax2, force);
}

void Hinge2Joint::setLowStop(float angle){
  dJointSetHinge2Param(mJoint, dParamLoStop, angle);
  dJointSetHinge2Param(mJoint, dParamLoStop, angle);
}

void Hinge2Joint::setHighStop(float angle){
  dJointSetHinge2Param(mJoint, dParamHiStop, angle);
  dJointSetHinge2Param(mJoint, dParamHiStop, angle);
}

void Hinge2Joint::setFudgeFactor(float factor){
  dJointSetHinge2Param(mJoint, dParamFudgeFactor, factor);
}

void Hinge2Joint::setSuspensionCFM(float factor){
  dJointSetHinge2Param(mJoint, dParamSuspensionCFM, factor);
}

void Hinge2Joint::setSuspensionERP(float factor){
  dJointSetHinge2Param(mJoint, dParamSuspensionERP, factor);
}

void Hinge2Joint::setStopCFM(float value){
  dJointSetHinge2Param(mJoint, dParamStopCFM, value);
}

void Hinge2Joint::setStopERP(float value){
  dJointSetHinge2Param(mJoint, dParamStopERP, value);
}

void Hinge2Joint::setSuspension(const Simulator& sim, float damping, float springiness){
  float erp = sim.mStepInterval*springiness/(sim.mStepInterval*springiness+damping);
  float cfm = 1.0f/(sim.mStepInterval*springiness+damping);
  setSuspensionERP(erp);
  setSuspensionCFM(cfm);
}

float Hinge2Joint::getAngleAxis1(){
  dReal ang = dJointGetHinge2Angle1(mJoint);
  return (float)ang;
}

void Hinge2Joint::addTorques(float torque1, float torque2){
  dJointAddHinge2Torques(mJoint, torque1, torque2);
}
