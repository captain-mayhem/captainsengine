#include "renderer/renderer.h"
#include "renderer/font.h"
#include "renderer/forms.h"

#include "common.h"
#include "textureManager.h"
#include "gamestate.h"
#include "camera.h"
#include "world.h"
#include "player.h"
#include "renderer.h"

#include <GL/gl.h>

HQRenderer* HQRenderer::rend_ = NULL;

HQRenderer::HQRenderer(Graphics::Renderer* rend){
  render_ = rend;
  
  //camera
  aspect_ = 1.0;
  fieldOV_ = 40;

  //start with 2D view
  threeD_ = false;
}

void HQRenderer::resize_(int width, int height){
  aspect_ = (float)width/height;
  render_->projection(fieldOV_, aspect_, 0.1f, 100.0f);
}


//! ascii input
void HQRenderer::ascii_(unsigned char key){
  switch (key){
  case 'y':
    break;
  }
}

void HQRenderer::initialize_(){
  //init textures
  TextureManager::init();

  render_->setClearColor(Vector3D());

  //init game
  game.init();

  //start game
  if (!game.start())
    EXIT();

}

void HQRenderer::paint_(){
  game.run();
  
  render_->clear(ZBUFFER | COLORBUFFER);
  render_->setColor(255,255,255,255);
  render_->resetModelView();

  if (wrld.isLoaded()){
    //number of moves in the upper right corner
    System::Engine::instance()->getFont()->glPrint(1000, 750, toStr(game.getMoves()).c_str(), 1);
    //get the nearest vertices and check them for camera collision
    Vector3D** worldCollision = wrld.getWorld();
    cam.checkCameraCollision(worldCollision, wrld.getNumberOfVerts());
    //allow lookat changes by mouse
    cam.look();
    //render world without blending
    glDisable(GL_BLEND);
    if (threeD_)
      wrld.render();
    else
      wrld.render2D(plyr.isZargon());
    glEnable(GL_BLEND);
  }
  
  render_->ortho(SCREENWIDTH, SCREENHEIGHT);
  render_->translate(-SCREENWIDTH/2, -SCREENHEIGHT/2, 0);
  render_->blendFunc(Graphics::BLEND_SRC_ALPHA, Graphics::BLEND_ONE);
  render_->enableBlend(true);
  
  if (!wrld.isLoaded()){
    TextureManager::instance()->otherTex[0]->activate();
    Graphics::Forms* form = System::Engine::instance()->getForms();
    render_->pushMatrix();
    render_->translate(512, 384, 0);
    render_->scale(1024, 768, 1);
    form->activateQuad();
    form->drawQuad();
    render_->popMatrix();
    /*
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
    */
  }
  
  Font *f = System::Engine::instance()->getFont();
  f->setColor(0,1,0);
  f->glPrint(20, 720, "Current Frames Per Second: "+toStr(System::Engine::instance()->getFPS()), 0);
  
  //f->render();
 
  
  render_->enableBlend(false);
  
}
