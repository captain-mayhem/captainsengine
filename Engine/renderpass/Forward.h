#ifndef CGE_FORWARD_RENDERING
#define CGE_FORWARD_RENDERING

#include <cge_export.h>

#include "Technique.h"
#include "mesh/mesh.h"

namespace CGE{

//! Draws the models lit by up to 8 of the scene's lights (the nearest ones to the camera)
class CGE_API ForwardRenderer : public RenderTechnique{
public:
  virtual void render(Gfx::DrawContext& ctx, const Gfx::PassDesc& output);
  //! draws a part of the scene into the current pass with depth testing
  /*! Leaves the blend mode as it is. */
  void render(Gfx::DrawContext& ctx, Mesh::DrawPart part);
};

}

#endif
