#ifndef CGE_RENDER_TECHNIQUE_H
#define CGE_RENDER_TECHNIQUE_H

#include <cge_export.h>

#include <cstdlib>

namespace CGE{

class Scene;

namespace Gfx{
class DrawContext;
struct PassDesc;
}

//! A way of rendering a scene with its camera and lights
/*! createResources, then render(ctx, output) per frame, and
    releaseResources before the device goes. */
class CGE_API RenderTechnique{
public:
  RenderTechnique() : mScene(nullptr) {}
  virtual ~RenderTechnique() {}
  virtual void setScene(Scene& scene) { mScene = &scene; }
  //! creates what the technique needs on the context's device
  virtual bool createResources(Gfx::DrawContext& ctx) { return true; }
  //! deletes it again; before the context and its device go
  virtual void releaseResources(Gfx::DrawContext& ctx) {}
  //! renders the scene into output's target, beginning and ending its pass
  /*! Outside a pass. Uses the scene's active camera, if it has one. */
  virtual void render(Gfx::DrawContext& ctx, const Gfx::PassDesc& output) = 0;
protected:
  Scene* mScene;
};

}

#endif
