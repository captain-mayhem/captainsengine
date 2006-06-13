#include "renderer/renderer.h"
#include "renderer/font.h"

#include "common.h"
#include "textureManager.h"
#include "world.h"
#include "renderer.h"

#include <GL/gl.h>

HQRenderer* HQRenderer::rend_ = NULL;

HQRenderer::HQRenderer(Graphics::Renderer* rend){

}

void HQRenderer::initialize_(){
  Graphics::Renderer* rend = System::Engine::instance()->getRenderer();
  
  TextureManager::init();

  rend->setClearColor(Vector3D());

}

void HQRenderer::paint_(){
  Graphics::Renderer* rend = System::Engine::instance()->getRenderer();
  rend->clear(ZBUFFER | COLORBUFFER);
  rend->setColor(255,255,255,255);
  rend->resetModelView();

  if (wrld.isLoaded()){
  
  }
  
  rend->ortho(SCREENWIDTH, SCREENHEIGHT);
  rend->translate(-SCREENWIDTH/2, -SCREENHEIGHT/2, 0);
  rend->blendFunc(Graphics::BLEND_SRC_ALPHA, Graphics::BLEND_ONE);
  rend->enableBlend(true);
  
  if (!wrld.isLoaded()){
    TextureManager::instance()->otherTex[0]->activate();
    glBegin(GL_QUADS);
      glTexCoord2f(1,0);
      glVertex2i(SCREENWIDTH, SCREENHEIGHT);
      glTexCoord2f(0,0);
      glVertex2i(0, SCREENHEIGHT);
      glTexCoord2f(0,1);
      glVertex2i(0, 0);
      glTexCoord2f(1,1);
      glVertex2i(SCREENWIDTH, 0); 
    glEnd();
  }
  
  System::Engine::instance()->getFont()->render();
  
  rend->enableBlend(false);
  
}
