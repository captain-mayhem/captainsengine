#ifndef CGE_LIGHT_PREPASS_H
#define CGE_LIGHT_PREPASS_H

#include <cge_export.h>

#include <memory>

#include "Technique.h"
#include "Forward.h"

namespace CGE{

//! Deferred lighting: the opaque surfaces' normals, colours and depth go
//! into a G-buffer, every light is added up in a light buffer, and the two
//! are composed into the output; transparent surfaces are drawn forward on top
class CGE_API LightPrepassRenderer : public RenderTechnique{
public:
  LightPrepassRenderer();
  virtual ~LightPrepassRenderer();
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
  //! the forward renderer for transparent parts
  ForwardRenderer mTransRend;
};

}

#endif
