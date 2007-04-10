#ifndef MATERIAL_H
#define MATERIAL_H

namespace Graphics{

struct Color{
  Color(){r = 1.0; g = 1.0; b = 1.0; a=1.0;}
  Color(float rn, float gn, float bn, float an){
    r = rn; g = gn; b = bn; a = an;
  }
  //Color(float rn, float gn, float bn, float an){
  //  r = rn*255; g = gn*255; b = bn*255; a = an*255;
  //}
  //unsigned char r, g, b, a;
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
