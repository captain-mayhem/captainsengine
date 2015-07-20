
#include "Simulator.h"

#include <cstdlib>

#include "Body.h"
#include "CollisionSpace.h"
#include "CollisionBody.h"
#include "CollisionPlane.h"

using namespace CGE;

Simulator::Simulator(float timeStep) : mSimulationCB(NULL), mCollisionCB(NULL){
  dInitODE2(0);
  dAllocateODEDataForThread(dAllocateMaskAll);
  mWorld = dWorldCreate();
  mSpace = new CollisionSpace(NULL, CollisionSpace::Hash);
  mStepInterval = timeStep;
  mTimeAccumulator = 0;
}

Simulator::~Simulator(){
  delete mSpace;
  dWorldDestroy(mWorld);
  dCleanupODEAllDataForThread();
  dCloseODE();
}

dJointGroupID contactgroup;
void Simulator::doSimulationStep(double time){
  mTimeAccumulator += (float)time;
  while (mTimeAccumulator >= mStepInterval){
    if (mSimulationCB)
      mSimulationCB(mStepInterval);
    dSpaceCollide(mSpace->mSpace, this, nearCallback);
    dWorldQuickStep(mWorld, mStepInterval);
    dJointGroupEmpty (contactgroup);
    mTimeAccumulator -= mStepInterval;
  }
}

void Simulator::nearCallback(void* data, dGeomID o1, dGeomID o2){
  Simulator* sim = (Simulator*)data;
  /*CollisionBase* c1;
  CollisionBase* c2;
  if (dGeomIsSpace(o1))
    c1 = CollisionSpace::getCollisionSpace((dSpaceID)o1);
  else
    c1 = (CollisionBase*)dGeomGetData(o1);
  if (dGeomIsSpace(o2))
    c2 = CollisionSpace::getCollisionSpace((dSpaceID)o2);
  else
    c2 = (CollisionBase*)dGeomGetData(o2);
  if (sim->mCollisionCB)
    sim->mCollisionCB(c1,c2);*/
  if (dGeomIsSpace(o1) || dGeomIsSpace(o2)){
    dSpaceCollide2(o1,o2,data,nearCallback);
    //if (dGeomIsSpace(o1))
    //  dSpaceCollide((dSpaceID)o1,data,nearCallback);
    //if (dGeomIsSpace(o2))
    //  dSpaceCollide((dSpaceID)o2,data,nearCallback);
    return;
  }
  CollisionObject* obj1 = (CollisionObject*)dGeomGetData(o1);
  CollisionObject* obj2 = (CollisionObject*)dGeomGetData(o2);
  /*dBodyID bdy = 0;
  CollisionPlane* p = dynamic_cast<CollisionPlane*>(obj1);
  if (p){
    bdy = dGeomGetBody(o2);
  }
  else{
    p = dynamic_cast<CollisionPlane*>(obj2);
    if (p){
      bdy = dGeomGetBody(o1);
    }
  }
  if (bdy == 0)
    return;
  if (dBodyGetType(bdy) == 1)
    return;*/
  //if (dGeomGetClass(o1) == dHeightfieldClass && dGeomGetClass(o2) != dCylinderClass)
    //return;
  //if (dGeomGetClass(o2) == dHeightfieldClass && dGeomGetClass(o1) != dCylinderClass)
  //  return;
  const int N = 10;
  dContact contact[N];
  int n = dCollide (o1,o2,N,&contact[0].geom,sizeof(dContact));
  for (int i=0; i<n; i++) {
    CollisionResponse resp(contact+i);
    bool handleCollision = false;
    if (sim->mCollisionCB)
      handleCollision = sim->mCollisionCB((CollisionObject*)dGeomGetData(contact[i].geom.g1),(CollisionObject*)dGeomGetData(contact[i].geom.g2),resp);
    //contact[i].surface.mode = dContactSlip1 | dContactSlip2 |
    //  dContactSoftERP | dContactSoftCFM | dContactApprox1;
    //contact[i].surface.mu = dInfinity;
    //contact[i].surface.slip1 = 0.0001;
    //contact[i].surface.slip2 = 0.00003;
    //contact[i].surface.soft_erp = 0.99;
    //contact[i].surface.soft_cfm = 0.001;
    //contact[i].surface.bounce_vel = 0.1;
    if (handleCollision){
      dJointID c = dJointCreateContact(sim->mWorld,contactgroup,&contact[i]);
      dJointAttach(c,
        dGeomGetBody(contact[i].geom.g1),
        dGeomGetBody(contact[i].geom.g2));
    }
  }
}

void Simulator::setGravitation(const Vec3f& vec){
  dWorldSetGravity(mWorld, vec.x, vec.y, vec.z);
  contactgroup = dJointGroupCreate(0);
}

//int Simulator::collide(CollisionBase* base1, CollisionBase* base2, CollisionResponse* resp){
  /*dGeomID o1;
  dGeomID o2;
  bool space = false;
  if (base1->getType() == CollisionBase::Space){
    o1 = (dGeomID)((CollisionSpace*)base1)->mSpace;
    space = true;
  }
  else{
    o1 = ((CollisionObject*)base1)->mGeom;
  }
  if (base2->getType() == CollisionBase::Space){
    o2 = (dGeomID)((CollisionSpace*)base2)->mSpace;
    space = true;
  }
  else{
    o2 = ((CollisionObject*)base2)->mGeom;
  }
  if (space){
    dSpaceCollide2(o1,o2,this,nearCallback);
    return 0;
  }

  return dCollide(o1,o2,0,&resp->geom,sizeof(dContact));*/
  //return 0;
//}
