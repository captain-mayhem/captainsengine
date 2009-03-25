#ifndef MATERIAL_H
#define MATERIAL_H

#include "../math/vector.h"

namespace Graphics{

struct Color{
  Color(){r = 1.0; g = 1.0; b = 1.0; a=1.0;}
  Color(float rn, float gn, float bn, float an){
    r = rn; g = gn; b = bn; a = an;
  }
  Color(const CGE::Vector3D& col){
    r = col.x; g = col.y; b = col.z; a = 1.0;
  }
  union{
    struct{
      float r;
      float g;
      float b;
      float a;
    };
    float array[4];
  };
};
  
class Material{
public:
  Material();
  ~Material();
#ifdef WIN32
#endif
  Color diffuse;
  Color ambient;
  Color specular;
  Color emissive;
  float power;
#ifdef WIN32
#endif
protected:
};

}

#endif
