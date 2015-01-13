#ifndef CGE_MATERIAL_H
#define CGE_MATERIAL_H

#include "../math/vector.h"

namespace CGE{

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
  void setDiffuse(const Color& diffuse) {mDiffuse = diffuse;}
  const Color& getDiffuse() const {return mDiffuse;}
  void setAmbient(const Color& ambient) {mAmbient = ambient;}
  const Color& getAmbient() const {return mAmbient;}
  void setSpecular(const Color& specular) {mSpecular = specular;}
  const Color& getSpecular() const {return mSpecular;}
  //void setEmissive(const Color& emissive) {mEmissive = emissive;}
  //const Color& getEmissive() const {return mEmissive;}
  void setPower(float power) {mPower = power;}
  const float& getPower() const {return mPower;}
protected:
  Color mDiffuse;
  Color mAmbient;
  Color mSpecular;
  float mPower;
};

}

#endif
