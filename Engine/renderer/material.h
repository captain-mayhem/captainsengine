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
  //! how a lit draw shades the material
  enum class Shading{
    //! Phong with the diffuse, ambient and specular colours and the shininess (BuiltinShader::Lit)
    Phong,
    //! glTF's metallic-roughness material (BuiltinShader::Pbr)
    /*! The diffuse colour and texture are the base colour, with the
        metalness, the roughness, the emissive colour and the
        metallic-roughness, normal, occlusion and emissive textures. The
        ambient colour is the light from everywhere, as with Phong. The
        Phong colours stay for what draws without it (the light prepass). */
    MetallicRoughness,
  };
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

  void setShading(Shading shading) { mShading = shading; }
  Shading getShading() const { return mShading; }
  bool isMetallicRoughness() const { return mShading == Shading::MetallicRoughness; }
  //! 0 for a dielectric, 1 for a metal (MetallicRoughness); 0 by default
  void setMetallic(float metallic) { mMetallic = metallic; }
  float getMetallic() const { return mMetallic; }
  //! 0 for a mirror, 1 for a fully rough surface (MetallicRoughness); 1 by default
  void setRoughness(float roughness) { mRoughness = roughness; }
  float getRoughness() const { return mRoughness; }
  //! the light the surface gives off (sRGB, like the other colours); black by default
  void setEmissive(const Color& emissive) { mEmissive = emissive; }
  const Color& getEmissive() const { return mEmissive; }
  //! the emissive colour (made linear) is multiplied with it; 1 by default
  void setEmissiveStrength(float strength) { mEmissiveStrength = strength; }
  float getEmissiveStrength() const { return mEmissiveStrength; }
  //! the metalness (blue) and the roughness (green), multiplied with the factors
  void setMetallicRoughnessTexture(std::shared_ptr<MaterialTexture> const& texture) { mMetallicRoughnessTexture = texture; }
  std::shared_ptr<MaterialTexture> const& getMetallicRoughnessTexture() const { return mMetallicRoughnessTexture; }
  //! a tangent-space normal map (glTF's: +x along u, +y up in the image)
  void setNormalTexture(std::shared_ptr<MaterialTexture> const& texture) { mNormalTexture = texture; }
  std::shared_ptr<MaterialTexture> const& getNormalTexture() const { return mNormalTexture; }
  //! the normal map's x and y are multiplied with it; 1 by default
  void setNormalScale(float scale) { mNormalScale = scale; }
  float getNormalScale() const { return mNormalScale; }
  //! how much of the light from everywhere reaches the surface (red)
  void setOcclusionTexture(std::shared_ptr<MaterialTexture> const& texture) { mOcclusionTexture = texture; }
  std::shared_ptr<MaterialTexture> const& getOcclusionTexture() const { return mOcclusionTexture; }
  //! 0: the occlusion texture has no effect, 1: its full effect (the default)
  void setOcclusionStrength(float strength) { mOcclusionStrength = strength; }
  float getOcclusionStrength() const { return mOcclusionStrength; }
  //! multiplied with the emissive colour
  void setEmissiveTexture(std::shared_ptr<MaterialTexture> const& texture) { mEmissiveTexture = texture; }
  std::shared_ptr<MaterialTexture> const& getEmissiveTexture() const { return mEmissiveTexture; }

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
  Shading mShading;
  float mMetallic;
  float mRoughness;
  Color mEmissive;
  float mEmissiveStrength;
  float mNormalScale;
  float mOcclusionStrength;
  std::shared_ptr<MaterialTexture> mMetallicRoughnessTexture;
  std::shared_ptr<MaterialTexture> mNormalTexture;
  std::shared_ptr<MaterialTexture> mOcclusionTexture;
  std::shared_ptr<MaterialTexture> mEmissiveTexture;
};

}

#endif
