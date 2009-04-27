
#include "light.h"

using namespace CGE;

Light::Light(Type t, const Vec3f val){
  mType = t;
  if (t == Directional)
    mDirection = val;
  else if (t == Point)
    mPosition = val;
}

Light::~Light(){

}