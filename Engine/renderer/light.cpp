
#include "light.h"

using namespace CGE;

Light::Light(Type t, const Vec3f val) : mSpotDirection(0,0,1), mSpotCutoff(180){
  mType = t;
  mPosition = val;
  if (t == Directional)
    mPosition.w = 0.0f;
  else if (t == Point)
    mPosition.w = 1.0f;
}

Light::Light(const Vec3f& pos, const Vec3f& dir, float cutoff) : mType(Spot), mPosition(pos),
mSpotDirection(dir), mSpotCutoff(cutoff)
{

}

Light::~Light(){

}