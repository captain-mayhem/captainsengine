#include "material.h"

using namespace Graphics;

Material::Material(){
  ambient = Color(0.2,0.2,0.2,1.0);
  diffuse = Color(0.6,0.6,0.6,1.0);
  specular = Color(0.0,0.0,0.0,0.0);
  emissive = Color(0.0,0.0,0.0,0.0);
  power = 0.0f;
}

Material::~Material(){
}
