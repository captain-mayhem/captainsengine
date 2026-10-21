#ifndef CGE_LIGHT_PREPASS_H
#define CGE_LIGHT_PREPASS_H

#include <cge_export.h>

#include <memory>

#include "Technique.h"
#include "Forward.h"

namespace CGE{

class RenderTarget;
class Shader;
class Light;
class Material;

//! Deferred lighting: the opaque surfaces' normals, colours and depth go
//! into a G-buffer, every light is added up in a light buffer, and the two
//! are composed into the output; transparent surfaces are drawn forward on top
class CGE_API LightPrepassRenderer : public RenderTechnique{
public:
  LightPrepassRenderer();
  virtual ~LightPrepassRenderer();
  virtual void init();
  virtual void deinit();
  virtual void render();
  virtual void setScene(Scene& scene) { mTransRend.setScene(scene); RenderTechnique::setScene(scene); }
  virtual bool createResources(Gfx::DrawContext& ctx);
  virtual void releaseResources(Gfx::DrawContext& ctx);
  //! the output target needs a depth buffer
  virtual void render(Gfx::DrawContext& ctx, const Gfx::PassDesc& output);
private:
  //! the render device's shaders, pipelines and buffers
  struct DeviceResources;
  //! (re)creates the G-buffer and the light buffer at the output's size
  bool createTargets(Gfx::RenderDevice& device, int width, int height);
  std::unique_ptr<DeviceResources> mDevice;
  // --- the old renderers ---
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
  //! the final buffer
  RenderTarget* mCompositingBuffer;
  //! the forward renderer for transparent parts
  ForwardRenderer mTransRend;
};

}

#endif
