
#include "Vehicle.h"

//#define _USE_MATH_DEFINES
#include "math.h"
#include "renderer/renderer.h"
#include "renderer/forms.h"
#include "system/engine.h"
#include <physics/Simulator.h>
#include <physics/Hinge2Joint.h>

using namespace CGE;

Vehicle::Vehicle(const CGE::Simulator& sim) : CollisionSpace(sim.getRootSpace(), CollisionSpace::Simple){
  mAutobody = new CGE::CollisionBody(sim);
  float startheight = 2;
  float length = 3;
  float height = 1.0;
  float width = 1.7;
  float chassis_weight = 1;
  float wheel_width = 0.165;
  float wheel_radius = 0.381;
  float wheel_weight = 0.2;

  mAutobody->initBox(*this, width, height, length, chassis_weight);
  mAutobody->setPosition(Vec3f(0,startheight+height/2,0));
  //init wheels
  for (int i = 0; i < 4; ++i){
    mWheels[i] = new CGE::CollisionBody(sim);
    mWheels[i]->initCylinder(*this, wheel_width, wheel_radius, wheel_weight);
  }
  Matrix wheelsLeft(Matrix::Rotation, Vec3f(0,0,1), M_PI/2);
  Matrix wheelsRight(Matrix::Rotation, Vec3f(0,0,1), -M_PI/2);
  Vec3f frontLeft = Vec3f(-width/2,startheight,-length/2);
  Vec3f frontRight = Vec3f(width/2,startheight,-length/2);
  Vec3f backLeft = Vec3f(-width/2,startheight,length/2);
  Vec3f backRight = Vec3f(width/2,startheight,length/2);
  mWheels[0]->setPosition(frontLeft);
  mWheels[0]->setOrientation(wheelsLeft);
  mWheels[1]->setPosition(frontRight);
  mWheels[1]->setOrientation(wheelsRight);
  mWheels[2]->setPosition(backLeft);
  mWheels[2]->setOrientation(wheelsLeft);
  mWheels[3]->setPosition(backRight);
  mWheels[3]->setOrientation(wheelsRight);

  //add hinges
  for (int i = 0; i < 4; ++i){
    mWheelHinges[i] = new CGE::Hinge2Joint(sim);
    mWheelHinges[i]->attach(mAutobody, mWheels[i]);
  }
  mWheelHinges[0]->setAnchor(frontLeft);
  mWheelHinges[1]->setAnchor(frontRight);
  mWheelHinges[2]->setAnchor(backLeft);
  mWheelHinges[3]->setAnchor(backRight);

  for (int i = 0; i < 4; ++i){
    mWheelHinges[i]->setAxis1(Vec3f(0,1,0));
    mWheelHinges[i]->setAxis2(Vec3f(1,0,0));
    mWheelHinges[i]->setSuspensionERP(0.4);
    mWheelHinges[i]->setSuspensionCFM(0.8);
  }

  mWheelHinges[2]->setLowStop(0);
  mWheelHinges[2]->setHighStop(0);
  mWheelHinges[3]->setLowStop(0);
  mWheelHinges[3]->setHighStop(0);
  
}

Vehicle::~Vehicle(){
  delete mAutobody;
  for (int i = 0; i < 4; ++i){
    delete mWheelHinges[i];
    delete mWheels[i];
  }
}

void Vehicle::setPosition(const CGE::Vec3f& pos){
  mAutobody->setPosition(pos);
}

void Vehicle::render(const CGE::Camera& cam){
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  CGE::Forms* form = CGE::Engine::instance()->getForms();

  CGE::Material mat;
  mat.setDiffuse(CGE::Color(1.0,1.0,0.0,1.0));
  mat.setAmbient(CGE::Color(0.7,0.7,0.0,1.0));
  rend->setMaterial(mat);
  mAutobody->render(cam);
  mat.setDiffuse(CGE::Color(0.0,0.0,1.0,1.0));
  mat.setAmbient(CGE::Color(0.0,0.0,0.3,1.0));
  rend->setMaterial(mat);
  for (int i = 0; i < 4; ++i){
    mWheels[i]->render(cam);
  }
}

void Vehicle::accelerate(float torque){
  for (int i = 0; i < 2; ++i){
    //mWheelHinges[i]->setVelocityAxis2(torque);
    //mWheelHinges[i]->setMaxForceAxis2(0.1);
    mWheelHinges[i]->addTorques(0, torque);
    mWheelHinges[i]->setFudgeFactor(0.1);
  }
}

void Vehicle::steer(float value){
  for (int i = 0; i < 2; ++i){
    float vel = value - mWheelHinges[i]->getAngleAxis1();
    mWheelHinges[i]->setVelocityAxis1(vel);
    mWheelHinges[i]->setMaxForceAxis1(0.2);
    mWheelHinges[i]->setLowStop(-0.75);
    mWheelHinges[i]->setHighStop(0.75);
  }
}