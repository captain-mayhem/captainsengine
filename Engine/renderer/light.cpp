
#include "light.h"

using namespace CGE;

Light::Light(Type t, const Vec3f val){
  mType = t;
  mPosition = val;
  if (t == Directional)
    mPosition.w = 0.0f;
  else if (t == Point)
    mPosition.w = 1.0f;
}

Light::~Light(){

}