#include "material.h"

using namespace CGE;

Material::Material(){
  mAmbient = Color(0.0f,0.0f,0.0f,1.0f);
  mDiffuse = Color(1.0f,1.0f,1.0f,1.0f);
  mSpecular = Color(0.0f,0.0f,0.0f,1.0f);
  mPower = 0.0f;
}

Material::~Material(){
}
