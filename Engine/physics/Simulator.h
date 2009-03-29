
#include <ode/ode.h>

#include "../math/vector.h"

namespace CGE{

class Body;
class CollisionSpace;

class Simulator{
  friend class Body;
  friend class Joint;
  friend class Hinge2Joint;
public:
  Simulator();
  ~Simulator();
  CollisionSpace* getRootSpace() const {return mSpace;}
  void setSimulationCallback(void (*func)(double)) {mSimulationCB = func;}
  void doSimulationStep(double time);
  void setGravitation(const Vec3f& vec);
protected:
  static void nearCallback(void* data, dGeomID o1, dGeomID o2);
  dWorldID mWorld;
  CollisionSpace* mSpace;
  void (*mSimulationCB)(double time);
};
}