
#include "Body.h"

#include <ode/ode.h>

#include "Simulator.h"

using namespace CGE;

Body::Body(const Simulator& simulator) /*: mMass(0), mCenterOfGravity(), mInertiaTensor(CGE::Matrix::Identity) */{
  mBody = dBodyCreate(simulator.mWorld);
}

Body::~Body(){
  dBodyDestroy(mBody);
}

void Body::initCylinder(float height, float radius, float mass){
  /*float r2_2 = radius*radius/2;
  *mInertiaTensor[0][0] = r2_2;
  *mInertiaTensor[1][1] = r2_2/2 + height/2*height/2/3;
  *mInertiaTensor[2][2] = *mInertiaTensor[1][1];
  mMass = mass;
  mInertiaTensor *= mMass;*/
  dMass m;
  dMassSetCylinderTotal(&m, mass, 2, radius, height);
  dBodySetMass(mBody, &m);
}

void Body::initBox(float width, float height, float depth, float mass){
  /*float a2 = width/2*width/2;
  float b2 = height/2*height/2;
  float c2 = depth/2*depth/2;
  *mInertiaTensor[0][0] = (b2+c2)/3;
  *mInertiaTensor[1][1] = (a2+c2)/3;
  *mInertiaTensor[2][2] = (a2+b2)/3;
  mMass = mass;
  mInertiaTensor *= mMass;*/
  dMass m;
  dMassSetBoxTotal(&m, mass, width, height, depth);
  dBodySetMass(mBody, &m);
}

void Body::initSphere(float radius, float mass){
  /*float t = 2*radius*radius/5;
  *mInertiaTensor[0][0] = t;
  *mInertiaTensor[1][1] = t;
  *mInertiaTensor[2][2] = t;
  mMass = mass;
  mInertiaTensor *= mMass;*/
  dMass m;
  dMassSetSphereTotal(&m, mass, radius);
  dBodySetMass(mBody, &m);
}

void Body::setPosition(const Vec3f& pos){
  dBodySetPosition(mBody, pos.x, pos.y, pos.z);
}

Vec3f Body::getPosition(){
  const dReal* tmp = dBodyGetPosition(mBody);
  return Vec3f((float)tmp[0],(float)tmp[1],(float)tmp[2]);
}

void Body::setOrientation(const Matrix& orientation){
  dMatrix3 mat;
  mat[0] = orientation.at(0);
  mat[1] = orientation.at(4);
  mat[2] = orientation.at(8);
  mat[3] = orientation.at(12);
  mat[4] = orientation.at(1);
  mat[5] = orientation.at(5);
  mat[6] = orientation.at(9);
  mat[7] = orientation.at(13);
  mat[8] = orientation.at(2);
  mat[9] = orientation.at(6);
  mat[10] = orientation.at(10);
  mat[11] = orientation.at(14);
  dBodySetRotation(mBody, mat);
}

Matrix Body::getOrientation(){
  const dReal* tmp = dBodyGetRotation(mBody);
  Matrix mat = Matrix(Matrix::Identity);
  mat.at(0) = (float)tmp[0];
  mat.at(4) = (float)tmp[1];
  mat.at(8) = (float)tmp[2];
  mat.at(12) = (float)tmp[3];
  mat.at(1) = (float)tmp[4];
  mat.at(5) = (float)tmp[5];
  mat.at(9) = (float)tmp[6];
  mat.at(13) = (float)tmp[7];
  mat.at(2) = (float)tmp[8];
  mat.at(6) = (float)tmp[9];
  mat.at(10) = (float)tmp[10];
  mat.at(14) = (float)tmp[11];
  return mat;
}