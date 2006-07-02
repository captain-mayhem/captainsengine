#include "renderer/renderer.h"
#include "renderer/font.h"
#include "renderer/forms.h"
#include "gui/gui.h"
#include "input/mouse.h"
#include "input/keyboard.h"

#include "common.h"
#include "textureManager.h"
#include "gamestate.h"
#include "camera.h"
#include "world.h"
#include "player.h"
#include "menu.h"
#include "message.h"
#include "renderer.h"

//#include <GL/gl.h>
using Gui::InputField;
using Gui::Button;
using Input::Mouse;

HQRenderer* HQRenderer::rend_ = NULL;

HQRenderer::HQRenderer(Graphics::Renderer* rend){
  render_ = rend;
  
  //camera
  aspect_ = 1.0;
  fieldOV_ = 40;

  //start with 2D view
  threeD_ = false;
  // the clicked field
  clickedField_ = Vector2D(-1,-1);
  awaitMapClick_ = NULL;

  inventory_ = NULL;
}

void HQRenderer::resize_(int width, int height){
  aspect_ = (float)width/height;
  render_->projection(fieldOV_, aspect_, 0.1f, 100.0f);
}

//special input
void HQRenderer::special_(int key){
  switch (key){
  case KEY_ESCAPE:
    msg.quit();
    break;
  }
}

// ascii input
void HQRenderer::ascii_(unsigned char key){
  switch (key){
  case 'y':
    break;
  case 'q':
  case 'm':
    if (game.getState() != RUN)
      break;
    threeD_ = !threeD_;
    if (threeD_ && wrld.isLoaded()){
      Mouse::instance()->showCursor(false);
    }
    else
      Mouse::instance()->showCursor(true);
    break;
  }
}


// the button click handling
void HQRenderer::buttonDown_(int x, int y, int buttons){
  //clickable map
  if (game.getState() == PREPARE && !Mouse::instance()->isGuiClick() && clickedField_ == Vector2D(-1,-1)){
    float dx = (float)wrld.getMapSize().x/SCREENWIDTH;
    float dy = (float)wrld.getMapSize().y/SCREENHEIGHT;
    clickedField_.x = (int)(x*dx);
    clickedField_.y = (int)(y*dy);

    //buttons to add a hero at this position
    System::Engine::instance()->getFont()->setColor(1,1,1);
    System::Engine::instance()->getFont()->glPrint(250, 500, "Hero name:", 1, (float)HUGE_VAL);
    InputField* in = new InputField();
    in->setPosition(Vector2D(400, 500));
    System::Engine::instance()->addInputListener(in);

    Button* but = new Button();
    but->setPosition(Vector2D(400, 450));
    but->setText("    Play");
    but->setCbFunc(Menu::play);
    System::Engine::instance()->addButtonListener(but);
  }
  else if (game.getState() == RUN && !Mouse::instance()->isGuiClick() && awaitMapClick_){
    //awaiting click on map for various ingame functions
    float dx = (float)wrld.getMapSize().x/SCREENWIDTH;
    float dy = (float)wrld.getMapSize().y/SCREENHEIGHT;
    Vector2D click;
    click.x = (int)(x*dx);
    click.y = (int)(y*dy);
    //call function that awaits the click
    awaitMapClick_(click);
    awaitMapClick_ = NULL;
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

  render_->projection(fieldOV_, aspect_, 0.1f, 100.0f);
  
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
    //glEnable(GL_TEXTURE_2D);
    if (threeD_)
      wrld.render();
    else
      wrld.render2D(!plyr.isZargon());
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
 
  //render inventory
  glLoadIdentity();
  render_->translate(-512, -384, 0);
  if (inventory_){
    inventory_->render();
    inventory_->update();
  }
  
  Graphics::Font *f = System::Engine::instance()->getFont();
  f->setColor(0,1,0);
  f->glPrint(20, 720, "Current Frames Per Second: "+toStr(System::Engine::instance()->getFPS()), 0);
  
  //interpolate
 
  render_->enableBlend(false);  
}

// set the view to 3D
void HQRenderer::setViewTo3D(bool threeD){
  threeD_ = threeD;
  if (threeD_ && wrld.isLoaded()){
    Mouse::instance()->showCursor(false);
  }
  else{
    Mouse::instance()->showCursor(true);
  }
}
