#ifndef CGE_RENDER_TECHNIQUE_H
#define CGE_RENDER_TECHNIQUE_H

#include <cge_export.h>

#include <cstdlib>

namespace CGE{

class Scene;

class CGE_API RenderTechnique{
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
