
#include "Joint.h"

#include "Body.h"

using namespace CGE;

Joint::Joint(const Simulator& sim) : mJoint(NULL) {

}

Joint::~Joint(){
}

void Joint::attach(const Body* body1, const Body* body2){
  dJointAttach(mJoint, body1->mBody, body2->mBody);
}