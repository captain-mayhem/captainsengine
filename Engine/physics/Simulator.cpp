
#include "Simulator.h"

#include <cstdlib>

#include "Body.h"
#include "CollisionSpace.h"
#include "CollisionBody.h"
#include "CollisionPlane.h"

using namespace CGE;

Simulator::Simulator() : mSimulationCB(NULL){
  dInitODE2(0);
  dAllocateODEDataForThread(dAllocateMaskAll);
  mWorld = dWorldCreate();
  mSpace = new CollisionSpace(NULL, CollisionSpace::Hash);
}

Simulator::~Simulator(){
  delete mSpace;
  dWorldDestroy(mWorld);
  dCleanupODEAllDataForThread();
  dCloseODE();
}

dJointGroupID contactgroup;
void Simulator::doSimulationStep(double time){
  if (time == 0)
    return;
  if (mSimulationCB)
    mSimulationCB(time);
  dSpaceCollide(mSpace->mSpace, this, nearCallback);
  dWorldStep(mWorld, time);
  dJointGroupEmpty (contactgroup);
}

void Simulator::nearCallback(void* data, dGeomID o1, dGeomID o2){
  Simulator* sim = (Simulator*)data;
  if (dGeomIsSpace(o1) || dGeomIsSpace(o2)){
    dSpaceCollide2(o1,o2,data,nearCallback);
    if (dGeomIsSpace(o1))
      dSpaceCollide((dSpaceID)o1,data,nearCallback);
    if (dGeomIsSpace(o2))
      dSpaceCollide((dSpaceID)o2,data,nearCallback);
    return;
  }
  /*CollisionObject* obj1 = (CollisionObject*)dGeomGetData(o1);
  CollisionObject* obj2 = (CollisionObject*)dGeomGetData(o2);
  dBodyID bdy = 0;
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
  if (dGeomGetClass(o1) == dHeightfieldClass && dGeomGetClass(o2) != dCylinderClass)
    return;
  if (dGeomGetClass(o2) == dHeightfieldClass && dGeomGetClass(o1) != dCylinderClass)
    return;
  const int N = 10;
  dContact contact[N];
  int n = dCollide (o1,o2,N,&contact[0].geom,sizeof(dContact));
  for (int i=0; i<n; i++) {
    contact[i].surface.mode = dContactSlip1 | dContactSlip2 |
      dContactSoftERP | dContactSoftCFM | dContactApprox1;
    contact[i].surface.mu = dInfinity;
    contact[i].surface.slip1 = 0.1;
    contact[i].surface.slip2 = 0.1;
    contact[i].surface.soft_erp = 0.5;
    contact[i].surface.soft_cfm = 0.3;
    dJointID c = dJointCreateContact(sim->mWorld,contactgroup,&contact[i]);
    dJointAttach(c,
      dGeomGetBody(contact[i].geom.g1),
      dGeomGetBody(contact[i].geom.g2));
  }
}

void Simulator::setGravitation(const Vec3f& vec){
  dWorldSetGravity(mWorld, vec.x, vec.y, vec.z);
  contactgroup = dJointGroupCreate(0);
}