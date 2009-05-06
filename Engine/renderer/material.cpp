#include "material.h"

using namespace CGE;

Material::Material(){
  mAmbient = Color(0.2,0.2,0.2,1.0);
  mDiffuse = Color(0.8,0.8,0.8,1.0);
  mSpecular = Color(0.0,0.0,0.0,1.0);
  mEmissive = Color(0.0,0.0,0.0,1.0);
  mPower = 0.0f;
}

Material::~Material(){
}
