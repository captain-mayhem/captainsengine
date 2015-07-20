#include "rendertarget.h"
#include <system/engine.h>
#include <renderer/renderer.h>
#include <renderer/forms.h>

using namespace CGE;

RenderTarget* RenderTarget::mCurrTarget = NULL;

void RenderTarget::drawFullscreen(bool transform){
  Renderer* rend = Engine::instance()->getRenderer();
  if (transform){
    rend->ortho(1, 1);
    rend->resetModelView();
  }
  rend->enableDepthTest(false);
  CGE::Forms& forms = *CGE::Engine::instance()->getForms();
  forms.activateQuad();
  rend->enableTexturing(true);
  rend->setColor(1, 1, 1, 1);
  if (transform){
    if (rend->getRenderType() != DirectX){
      rend->switchMatrixStack(CGE::MatTexture);
      rend->resetModelView();
      rend->scale(1, -1, 1);
    }
  }
  for (unsigned i = 0; i < getNumTextures(); ++i){
    getTexture(i)->activate(i);
  }
  forms.drawQuad(Vec2f(), Vec2f(1, 1));
  if (transform){
    if (rend->getRenderType() != DirectX){
      rend->resetModelView();
      rend->switchMatrixStack(CGE::Modelview);
    }
  }
}
