#ifndef CGE_MATERIAL_H
#define CGE_MATERIAL_H

#include <cge_export.h>

#include <string>
#include <vector>
#include "../math/vector.h"

namespace CGE{

struct CGE_API Color{
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
  
class CGE_API Material{
public:
  Material(std::string const& name);
  ~Material();
  void setDiffuse(const Color& diffuse) {mDiffuse = diffuse;}
  const Color& getDiffuse() const {return mDiffuse;}
  void setAmbient(const Color& ambient) {mAmbient = ambient;}
  const Color& getAmbient() const {return mAmbient;}
  void setSpecular(const Color& specular) {mSpecular = specular;}
  const Color& getSpecular() const {return mSpecular;}
  void setPower(float power) {mSpecPower = power;}
  const float& getPower() const {return mSpecPower;}
  void setOpacity(float opacity) { mOpacity = opacity; }
  const float getOpacity() const { return mOpacity; }
  //! the files of the textures, for loading them on a render device
  void setDiffuseTexFile(std::string const& file) { mDiffuseTexFile = file; }
  std::string const& getDiffuseTexFile() const { return mDiffuseTexFile; }
  void setOpacityTexFile(std::string const& file) { mOpacityTexFile = file; }
  std::string const& getOpacityTexFile() const { return mOpacityTexFile; }

  bool isTransparent() const { return mOpacity != 1.0f || !mOpacityTexFile.empty(); }
  
  std::string const & getName() const { return mName; }

  //! adds the materials of an MTL file; the textures are loaded when a mesh is drawn
  static bool loadFromMTL(std::string const& file, std::vector<Material*>& materials);
protected:
  std::string mName;
  Color mDiffuse;
  Color mAmbient;
  Color mSpecular;
  float mSpecPower;
  float mOpacity;
  std::string mDiffuseTexFile;
  std::string mOpacityTexFile;
};

}

#endif
