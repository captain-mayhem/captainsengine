#ifndef CGE_LIGHT_PREPASS_H
#define CGE_LIGHT_PREPASS_H

#include "Technique.h"

namespace CGE{

class RenderTarget;
class Shader;

class LightPrepassRenderer : public RenderTechnique{
public:
  virtual void init();
  virtual void deinit();
  virtual void render();
private:
  //! the base pass shader
  Shader* mBaseShader;
  //! the fbo
  RenderTarget* mGBuffer;
  //! the lighting pass
  Shader* mLightShader;
  int mCamNearLoc;
  int mCamFarLoc;
};

}

#endif
