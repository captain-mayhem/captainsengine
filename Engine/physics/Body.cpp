
#include "Body.h"

using namespace CGE;

Body::Body() /*: mMass(0), mCenterOfGravity(), mInertiaTensor(CGE::Matrix::Identity) */{

}

Body::~Body(){

}

void Body::initCylinder(float height, float radius, float mass){
  /*float r2_2 = radius*radius/2;
  *mInertiaTensor[0][0] = r2_2;
  *mInertiaTensor[1][1] = r2_2/2 + height/2*height/2/3;
  *mInertiaTensor[2][2] = *mInertiaTensor[1][1];
  mMass = mass;
  mInertiaTensor *= mMass;*/
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
}

void Body::initSphere(float radius, float mass){
  /*float t = 2*radius*radius/5;
  *mInertiaTensor[0][0] = t;
  *mInertiaTensor[1][1] = t;
  *mInertiaTensor[2][2] = t;
  mMass = mass;
  mInertiaTensor *= mMass;*/
}