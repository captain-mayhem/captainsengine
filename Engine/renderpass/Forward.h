#ifndef CGE_FORWARD_RENDERING
#define CGE_FORWARD_RENDERING

#include <cge_export.h>

#include "Technique.h"
#include "mesh/mesh.h"

namespace CGE{

class CGE_API ForwardRenderer : public RenderTechnique{
public:
  virtual void render(){
    render(Mesh::DRAW_ALL);
  }

  void render(Mesh::DrawPart part);
};

}

#endif