#ifndef CGE_FORWARD_RENDERING
#define CGE_FORWARD_RENDERING

#include "Technique.h"

namespace CGE{

class ForwardRenderer : public RenderTechnique{
public:
  virtual void render();
};

}

#endif