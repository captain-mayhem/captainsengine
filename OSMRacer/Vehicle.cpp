
#include "Vehicle.h"

//#define _USE_MATH_DEFINES
#include "math.h"
#include "renderer/renderer.h"
#include "renderer/forms.h"
#include "system/engine.h"
#include <physics/Simulator.h>
#include <physics/Hinge2Joint.h>
#include <renderer/font.h>

using namespace CGE;
using std::max;

Vehicle::Vehicle(const CGE::Simulator& sim) : CollisionSpace(sim.getRootSpace(), CollisionSpace::Simple){
  mAutobody = new CGE::CollisionBody(sim);
  float startheight = 2;
  float length = 3;
  float height = 1.0;
  float width = 1.7;
  float chassis_weight = 1500;
  float wheel_width = 0.165;
  float wheel_radius = 0.381;
  float wheel_weight = 7.0;
  float damping = 10000.0;
  float springiness = 125000.0;

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
    mWheelHinges[i]->setAnchor(*mWheels[i]);
  }
  //mWheelHinges[0]->setAnchor(frontLeft);
  //mWheelHinges[1]->setAnchor(frontRight);
  //mWheelHinges[2]->setAnchor(backLeft);
  //mWheelHinges[3]->setAnchor(backRight);

  for (int i = 0; i < 4; ++i){
    mWheelHinges[i]->setAxis1(Vec3f(0,1,0));
    mWheelHinges[i]->setAxis2(Vec3f(1,0,0));
    mWheelHinges[i]->setSuspension(sim, damping, springiness);
    //mWheelHinges[i]->setStopERP(0.001);
    //mWheelHinges[i]->setMaxForceAxis2(100000);
    //mWheelHinges[i]->setMaxForceAxis1(0);
  }

  //steering wheels
  for (int i = 0; i < 2; ++i){
    mWheelHinges[i]->setLowStop(-0.75);
    mWheelHinges[i]->setHighStop(0.75);
    mWheelHinges[i]->setMaxForceAxis1(1000);
    //mWheelHinges[i]->setStopERP(0.99);
    //mWheelHinges[i]->setStopCFM(0.01);
  }

  //back wheels
  for (int i = 2; i < 4; ++i){
    mWheelHinges[i]->setLowStop(0);
    mWheelHinges[i]->setHighStop(0);
    mWheelHinges[i]->setStopERP(0.99);
  }
  
}

Vehicle::~Vehicle(){
  delete mAutobody;
  for (int i = 0; i < 4; ++i){
    delete mWheelHinges[i];
    delete mWheels[i];
  }
}

CGE::Vec3f Vehicle::getPosition(){
  return mAutobody->getPosition();
}

CGE::Vec3f Vehicle::getDirection(){
  CGE::Matrix mat = mAutobody->getOrientation();
  return Vec3f(mat.at(8), mat.at(9), mat.at(10));
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

void Vehicle::simulate(float acceleration, float steering){
  char tmp[1024];
  //steering
  float velocity = mAutobody->getLinearVelocity().length()*3.6f;
  float factor = steering/(0.02f*velocity+1.0f);
  sprintf(tmp, "Velocity: %f", velocity);
  CGE::Engine::instance()->getFont(0)->print(10,480,tmp,0.1);
  for (int i = 0; i < 2; ++i){
    mWheelHinges[i]->setLowStop(0.75*factor);
    mWheelHinges[i]->setHighStop(0.75*factor);

    //float v = 12*steering - mWheelHinges[i]->getAngleAxis1();	
    //if (v > 0.1f) v = 0.1f;
    //if (v < -0.1f) v = -0.1f;
    //v *= 10.0f;
    //mWheelHinges[i]->setVelocityAxis1(v);

    //mWheelHinges[i]->addTorques(vel, 0);
    sprintf(tmp, "Steering: %f", factor);
    CGE::Engine::instance()->getFont(0)->print(10,500+20*i,tmp,0.1);
  }
  for (int i = 0; i < 4; ++i){
    mWheelHinges[i]->setMaxForceAxis2(0.0);
  }
  //acceleration
  for (int i = 0; i < 2; ++i){
    mWheelHinges[i]->setVelocityAxis2(acceleration);
    mWheelHinges[i]->setMaxForceAxis2(10000.0);
    //mWheelHinges[i]->addTorques(0, acceleration);
    //mWheelHinges[i]->setFudgeFactor(0.1);
    //mWheelHinges[i]->setVelocityAxis2()
  }
  //bound rolling
  Vec3f angvel = mAutobody->getAngularVelocity();
  if (angvel.length() > 5.0f){
    angvel.normalize();
    angvel *= 5.0f;
    mAutobody->setAngularVelocity(angvel);
  }

  Matrix orientation = mAutobody->getOrientation();
  //get rid of translation if there is any
  orientation.at(12) = 0; orientation.at(13) = 0; orientation.at(14) = 0;
  Matrix invorient = orientation;
  invorient = invorient.transpose(); //now, inversion can be done via transpose
  invorient.at(3) = 0; invorient.at(7) = 0; invorient.at(11) = 0;
  Vec3f invvel = invorient*angvel;
  Vec3f front = Vec3f(orientation.at(8), orientation.at(9), orientation.at(10));
  Vec3f side = Vec3f(orientation.at(0), orientation.at(1), orientation.at(2));
  Vec3f up = Vec3f(orientation.at(4), orientation.at(5), orientation.at(6));
  if (side.y * invvel.z > 0.0f) {
    const float lborder = 0.85f;
		float scale = max(-10.0f, (lborder - up.y) / (lborder - 1.0f));
		invvel.z *= (1.0f - scale) * fabs(front.y) + scale;
	}
  angvel = orientation*invvel;
  mAutobody->setAngularVelocity(angvel);

}
