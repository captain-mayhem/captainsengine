#ifndef CGE_RENDER_TECHNIQUE_H
#define CGE_RENDER_TECHNIQUE_H

#include <cstdlib>

namespace CGE{

class Scene;

class RenderTechnique{
public:
  RenderTechnique() : mScene(NULL) {}
  virtual void setScene(Scene& scene) { mScene = &scene; }
  virtual void init() {}
  virtual void deinit() {}
  virtual void render() = 0;
protected:
  Scene* mScene;
};

}

#endif
