#ifndef _GAMEOBJECT_H_
#define _GAMEOBJECT_H_

#include <math/vector.h>

class GameObject{
public:
  virtual CGE::Vec3f getPosition()=0;
  virtual CGE::Vec3f getDirection()=0;
};

#endif
