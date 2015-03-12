#ifndef CGE_LIGHT_PREPASS_H
#define CGE_LIGHT_PREPASS_H

#include "Technique.h"

namespace CGE{

class RenderTarget;
class Shader;
class Light;
class Material;

class LightPrepassRenderer : public RenderTechnique{
public:
  virtual void init();
  virtual void deinit();
  virtual void render();
private:
  static void applyMaterial(Shader* shader, Material const& mat, void* userdata);
  static void applyLight(Shader* shader, int number, Light const& light, void* userdata);
  //! the base pass shader
  Shader* mBaseShader;
  int mColorLoc;
  int mAmbientLoc;
  int mSpecularLoc;
  int mTexEnabledLoc;
  int mShininessLoc;
  //! the fbo
  RenderTarget* mGBuffer;
  //! the lighting pass
  Shader* mLightShader;
  int mCamNearLoc;
  int mCamFarLoc;
  int mLightPosLoc;
  int mLightDirLoc;
  int mLightColorLoc;
  int mLightCutoffLoc;
  int mLightAttenuationLoc;
  int mNearPlaneSizeLoc;
  //! the light buffer
  RenderTarget* mLightBuffer;
  //! the compositing shader
  Shader* mCompositingShader;
};

}

#endif
