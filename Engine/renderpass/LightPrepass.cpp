#include "LightPrepass.h"

#include <renderer/scene.h>
#include <renderer/rendertarget.h>
#include <renderer/renderer.h>
#include <system/engine.h>
#include <window/window.h>

using namespace CGE;

void LightPrepassRenderer::init(){
  Renderer* rend = Engine::instance()->getRenderer();
  mGBuffer = rend->createRenderTarget(rend->getWindow()->getWidth(), rend->getWindow()->getHeight());
  if (mGBuffer){
    mGBuffer->activate();
    mGBuffer->addTexture(CGE::Texture::RGBA);
    mGBuffer->addTexture(CGE::Texture::DEPTH);
    mGBuffer->isComplete();
    mGBuffer->deactivate();
  }
}

void LightPrepassRenderer::deinit(){
  delete mGBuffer;
}

void LightPrepassRenderer::render(){
  if (!mScene)
    return;
  mScene->render();
}