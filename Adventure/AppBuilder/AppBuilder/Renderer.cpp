#include "Renderer.h"

RND_CLS* AdvRenderer::mInstance = NULL;

void AdvRenderer::init(){
#ifdef RENDER_TEGRA
  mInstance = new GL2Renderer();
#else
  mInstance = new GL1Renderer();
#endif
}

void AdvRenderer::deinit(){
  delete mInstance;
}