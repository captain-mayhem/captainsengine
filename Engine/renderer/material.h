#ifndef CGE_MATERIAL_H
#define CGE_MATERIAL_H

#include <vector>
#include "../math/vector.h"

namespace CGE{

class Texture;

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
  Material(std::string const& name);
  ~Material();
  void setDiffuse(const Color& diffuse) {mDiffuse = diffuse;}
  const Color& getDiffuse() const {return mDiffuse;}
  void setDiffuseTex(Texture* tex) { mDiffuseTex = tex; }
  Texture const* getDiffuseTex() const { return mDiffuseTex; }
  void setAmbient(const Color& ambient) {mAmbient = ambient;}
  const Color& getAmbient() const {return mAmbient;}
  void setSpecular(const Color& specular) {mSpecular = specular;}
  const Color& getSpecular() const {return mSpecular;}
  void setPower(float power) {mSpecPower = power;}
  const float& getPower() const {return mSpecPower;}
  void setOpacity(float opacity) { mOpacity = opacity; }
  const float getOpacity() const { return mOpacity; }
  std::string const & getName() const { return mName; }
  static bool loadFromMTL(std::string const& file, std::vector<Material*>& materials, std::vector<Texture*>& textures);
protected:
  std::string mName;
  Color mDiffuse;
  Texture* mDiffuseTex;
  Color mAmbient;
  Color mSpecular;
  float mSpecPower;
  float mOpacity;
};

}

#endif
