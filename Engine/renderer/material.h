#ifndef CGE_MATERIAL_H
#define CGE_MATERIAL_H

#include <cge_export.h>

#include <memory>
#include <string>
#include <vector>
#include "../math/vector.h"

namespace CGE{

class Image;
namespace Gfx{
class RenderDevice;
class Texture;
}

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
  
//! an image for the textures of materials, which share it, and its texture on a render device
/*! E.g. an image embedded in a model file. The texture is made when it is
    first needed; the image is freed then. */
class CGE_API MaterialTexture{
public:
  //! name: for messages, e.g. the file or the image's name in a model file
  MaterialTexture(std::unique_ptr<Image> image, std::string const& name);
  ~MaterialTexture();
  MaterialTexture(const MaterialTexture&) = delete;
  MaterialTexture& operator=(const MaterialTexture&) = delete;
  std::string const& getName() const { return mName; }
  //! the image until the texture is made, then nullptr
  const Image* getImage() const { return mImage.get(); }
  //! the texture on device (RGBA8 with mipmaps), made at the first call; nullptr if that failed
  /*! The texture belongs to the first device asked; other devices get nullptr. */
  Gfx::Texture* getTexture(Gfx::RenderDevice& device);
private:
  std::string mName;
  std::unique_ptr<Image> mImage;
  std::unique_ptr<Gfx::Texture> mTexture;
  Gfx::RenderDevice* mDevice;
};

class CGE_API Material{
public:
  //! how the alpha of the diffuse colour and texture is used
  enum class AlphaMode{
    //! blended if the opacity is below 1 or there is an opacity texture (as in MTL files)
    Default,
    //! opaque; the alpha is ignored
    Opaque,
    //! opaque; pixels with an alpha below the cutoff are left out
    Mask,
    //! blended: drawn with the transparent part (Mesh::DRAW_TRANSPARENT)
    Blend,
  };
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

  //! the diffuse texture from an image in memory; it wins over the texture files
  void setDiffuseTexture(std::shared_ptr<MaterialTexture> const& texture) { mDiffuseTexture = texture; }
  std::shared_ptr<MaterialTexture> const& getDiffuseTexture() const { return mDiffuseTexture; }
  void setAlphaMode(AlphaMode mode) { mAlphaMode = mode; }
  AlphaMode getAlphaMode() const { return mAlphaMode; }
  //! the alpha below which AlphaMode::Mask leaves pixels out
  void setAlphaCutoff(float cutoff) { mAlphaCutoff = cutoff; }
  float getAlphaCutoff() const { return mAlphaCutoff; }
  //! drawn from both sides (without culling), the back lit as the front
  void setDoubleSided(bool doubleSided) { mDoubleSided = doubleSided; }
  bool isDoubleSided() const { return mDoubleSided; }

  bool isTransparent() const {
    if (mAlphaMode == AlphaMode::Default)
      return mOpacity != 1.0f || !mOpacityTexFile.empty();
    return mAlphaMode == AlphaMode::Blend;
  }
  
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
  std::shared_ptr<MaterialTexture> mDiffuseTexture;
  AlphaMode mAlphaMode;
  float mAlphaCutoff;
  bool mDoubleSided;
};

}

#endif
