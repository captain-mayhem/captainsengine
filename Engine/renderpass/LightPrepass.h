#ifndef CGE_LIGHT_PREPASS_H
#define CGE_LIGHT_PREPASS_H

#include "Technique.h"

namespace CGE{

class RenderTarget;

class LightPrepassRenderer : public RenderTechnique{
public:
  virtual void init();
  virtual void deinit();
  virtual void render();
private:
  //! the fbo
  RenderTarget* mGBuffer;
};

}

#endif
