
#include <ode/ode.h>

#include "../math/vector.h"

namespace CGE{

class Body;
class CollisionSpace;
class CollisionBase;
class CollisionObject;

class CollisionResponse{
public:
  CollisionResponse(dContact* contact) : mContact(contact) {contact->surface.mode = dContactApprox1;}
  void setMu(float mu) {mContact->surface.mu = mu;}
  void setSlip1(float slip) {mContact->surface.slip1 = slip; mContact->surface.mode |= dContactSlip1;}
  void setSlip2(float slip) {mContact->surface.slip2 = slip; mContact->surface.mode |= dContactSlip2;}
  void setSoftERP(float erp) {mContact->surface.soft_erp = erp; mContact->surface.mode |= dContactSoftERP;}
  void setSoftCFM(float cfm) {mContact->surface.soft_cfm = cfm; mContact->surface.mode |= dContactSoftCFM;}
  void setBounce(float bounce_vel) {mContact->surface.bounce_vel = bounce_vel;}
protected:
  dContact* mContact;
};

class Simulator{
  friend class Body;
  friend class Joint;
  friend class Hinge2Joint;
public:
  Simulator(float timeStep);
  ~Simulator();
  CollisionSpace* getRootSpace() const {return mSpace;}
  void setSimulationCallback(void (*func)(double)) {mSimulationCB = func;}
  void setCollisionCallback(bool (*func)(CollisionObject*,CollisionObject*,CollisionResponse&)) {mCollisionCB = func;}
  void doSimulationStep(double time);
  //int collide(CollisionBase* base1, CollisionBase* base2, CollisionResponse* resp);
  void setGravitation(const Vec3f& vec);
protected:
  static void nearCallback(void* data, dGeomID o1, dGeomID o2);
  dWorldID mWorld;
  CollisionSpace* mSpace;
  void (*mSimulationCB)(double time);
  bool (*mCollisionCB)(CollisionObject* a, CollisionObject* b, CollisionResponse& collision);
  float mStepInterval;
  float mTimeAccumulator;
};
}

