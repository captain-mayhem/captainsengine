
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
  float startheight = 1;
  float length = 3;
  float height = 1.5;
  float width = 1.7;
  float chassis_weight = 2000;
  float wheel_width = 0.165;
  float wheel_radius = 0.381;
  float wheel_weight = 5;

  mAutobody->initBox(*this, length, height, width, chassis_weight);
  mAutobody->setPosition(Vec3f(0,startheight,0));
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
    mWheelHinges[i]->setAxis1(Vec3f(0,1,0));
    mWheelHinges[i]->setAxis2(Vec3f(1,0,0));
  }
  mWheelHinges[0]->setAnchor(frontLeft);
  mWheelHinges[1]->setAnchor(frontRight);
  mWheelHinges[2]->setAnchor(backLeft);
  mWheelHinges[3]->setAnchor(backRight);
  
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

void Vehicle::render(const Graphics::Camera& cam){
  Graphics::Renderer* rend = System::Engine::instance()->getRenderer();
  Graphics::Forms* form = System::Engine::instance()->getForms();

  rend->setColor(0.0,0.0,1.0,1.0);
  for (int i = 0; i < 4; ++i){
    mWheels[i]->render(cam);
  }
}

void Vehicle::accelerate(float torque){
  for (int i = 0; i < 4; ++i){
    mWheelHinges[0]->setVelocityAxis2(torque);
    mWheelHinges[0]->setMaxForceAxis2(0.1);
  }
}