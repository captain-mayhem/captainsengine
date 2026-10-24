#include "renderer/screentext.h"
#include "renderer/forms.h"
#include "renderer/drawcontext.h"
#include "renderer/renderdevice.h"
#include "renderer/shaderlibrary.h"
#include "math/matrixstack.h"
#include "io/Tracing.h"
#include "input/mouse.h"
#include "input/keyboard.h"
#include "window/window.h"

#include "widgets.h"
#include "common.h"
#include "opcodes.h"
#include "textureManager.h"
#include "gamestate.h"
#include "camera.h"
#include "world.h"
#include "player.h"
#include "menu.h"
#include "message.h"
#include "trade.h"
#include "renderer.h"
#include "registerHeroquestShaders.h"

#define line *CGE::Engine::instance()->getScreenText(0)

using Gui::InputField;
using Gui::Button;
using Input::Mouse;

TR_CHANNEL(HQ_Renderer);

HQRenderer* HQRenderer::rend_ = nullptr;

HQRenderer::HQRenderer(){
  ask_ = false;

  //camera
  aspect_ = 1.0;
  fieldOV_ = 40;

  for (int i = 0; i < 256; i++){
    keys_[i] = false;
  }
  
  //start with 2D view
  threeD_ = false;
  // the clicked field
  clickedField_ = Vector2D(-1,-1);
  awaitMapClick_ = nullptr;
  mousePos_ = Vector2D(0,0);

  inventory_ = nullptr;
  trade_ = false;
}

HQRenderer::~HQRenderer(){
}

void HQRenderer::releaseResources(){
  if (!rend_)
    return;
  //the draw context's pipelines refer to the shader
  if (rend_->fogShader_)
    CGE::Engine::instance()->getDrawContext()->forgetShader(rend_->fogShader_.get());
  rend_->fogShader_.reset();
  TextureManager::release();
  wrld.releaseResources();
}

void HQRenderer::resize_(int width, int height){
  aspect_ = (float)width/height;
}

//special input
void HQRenderer::special_(int key){
  switch (key){
  case KEY_ESCAPE:
    if (inventory_){
      Menu::close(nullptr);
      break;
    }
    line << "Do you really want to quit (y/n)";
    ask_ = true;
    break;
  case KEY_W:
  case KEY_S:
  case KEY_A:
  case KEY_D:
    keys_[key] = true;
  }
}

//key up
void HQRenderer::keyUp_(int key){
  switch (key){
  case KEY_W:
  case KEY_S:
  case KEY_A:
  case KEY_D:
    keys_[key] = false;
  }
}

// handle keys continously
void HQRenderer::handleKeys(){
  float speed = (float)(cam.getSpeed()*CGE::Engine::instance()->getFrameInterval());
  if (keys_[KEY_W])
    cam.moveCamera(speed);
  if (keys_[KEY_S])
    cam.moveCamera(-speed);
  if (keys_[KEY_A])
    cam.strafeCamera(-speed);
  if (keys_[KEY_D])
    cam.strafeCamera(speed);
}

// ascii input
void HQRenderer::ascii_(unsigned char key){
  switch (key){
  case 'y':
    //quit application
    if (ask_)
      msg.quit();
    break;
  case 'n':
    ask_ = false;
    line << " ";
    break;
  case 'f':
    //lock/unlock mouse pointer in 3D
    if (threeD_){
      Mouse::instance()->showCursor(!Mouse::instance()->isMouseActive());
    }
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
  case 'c':{
    //change camera
    //only if it is not your turn or if no inventory is open
    if (plyr.yourTurn() || inventory_)
      break;
    //get next controlled creature
    Vector2D pos = game.getNextCreaturePos();
    //save old camera and set it to new
    Creature* creat = plyr.getCreature();
    if (creat){
		  creat->setLookAt(cam.view());
		  creat->setCamPos(cam.position());
		  creat->setRot(cam.getCurrRotX());
    }

    Creature* c = dynamic_cast<Creature*>(wrld.getObject(pos));
    if (!c)
      break;
		cam.positionCamera(c->getCamPos(), c->getLookAt(), Vector3D(0,1,0));
		cam.setCurrRotX(c->getRot());
		wrld.updateCollisionVertices(c->getPosition());
    plyr.setActCreature(c->getName());
    //notify server
    msg.send(std::to_string(CREAT_CHANGE)+" "+std::to_string(pos.x)+" "+std::to_string(pos.y));
    break;
  }
  case 'i':
    if (game.getState() == RUN)
      Menu::inventory(nullptr);
    break;
  case 'w':
  case 's':
  case 'a':
  case 'd':
    if (!threeD_){
      //no moves left?
      if (game.getMoves() <= 0)
        break;
      Vector2D pos = plyr.getCreature()->getPosition();
      Direction d;
      Vector2D moveTo;
      if (key == 'w'){
        d = TOP;
        moveTo = Vector2D(pos.x, pos.y-1);
      }
      else if (key == 's'){
        d = BOTTOM;
        moveTo = Vector2D(pos.x, pos.y+1);
      }
      else if (key == 'a'){
        d = LEFT;
        moveTo = Vector2D(pos.x-1, pos.y);
      }
      else if (key == 'd'){
        d = RIGHT;
        moveTo = Vector2D(pos.x+1, pos.y);
      }
      //is move not allowed due to wall?
      if (wrld.isWall(pos.x, pos.y, d))
        break;
      //is move not allowed due to other reasons?
	    bool selfHero = dynamic_cast<Hero*>(wrld.getObject(pos)) == nullptr ? false : true;
      bool objectHero = true;
	    bool objectMonster = true;
      bool objectFurni = false;
      bool overlayNotWalkable = false;
	    
      GameObject *o = wrld.getObject(moveTo);
	    if (o){
		    objectHero = dynamic_cast<Hero*>(o) == nullptr ? false : true;
		    objectMonster = dynamic_cast<Monster*>(o) == nullptr ? false : true;
        objectFurni = dynamic_cast<Furniture*>(o) == nullptr ? false : true;
	    }
      Field* f = &wrld.getField(moveTo);
      if (f->overlay && f->overlay->getStatus() && !f->overlay->isWalkable())
        overlayNotWalkable = true;
	
	    if (((selfHero && !objectHero) && wrld.getRespectClasses()) || ((!selfHero && !objectMonster) && wrld.getRespectClasses()) || objectFurni || overlayNotWalkable){
	      break;
      }

      char tmp[2];
      tmp[0] = key;
      tmp[1] = '\0';
      cam.moveTo(QUADSIZE, Vector3D(moveTo-pos));
      msg.process(tmp);
    }
    break;
  }
}


// the button click handling
void HQRenderer::buttonDown_(int x, int y, int buttons){
  //clickable map
  const CGE::VirtualScreen& screen = CGE::Engine::instance()->getVirtualScreen();
  //clicks beside the 4:3 area of a wider or narrower window are outside the map
  if (x < 0 || x >= screen.getWidth() || y < 0 || y >= screen.getHeight())
    return;
  if (game.getState() == PREPARE && !Mouse::instance()->isGuiClick() && clickedField_ == Vector2D(-1,-1) && !trade_){
    float dx = (float)wrld.getMapSize().x/screen.getWidth();
    float dy = (float)wrld.getMapSize().y/screen.getHeight();
    clickedField_.x = (int)(x*dx);
    clickedField_.y = (int)(y*dy);

    //buttons to add a hero at this position
    CGE::Engine::instance()->getScreenText(0)->setColor(1,1,1);
    CGE::Engine::instance()->getScreenText(0)->print(250, 500, "Hero name:", 1, (float)HUGE_VAL);
    InputField* in = new InputField();
    in->setName("Playname");
    in->setPosition(Vector2D(400, 500));
    widgets.add(in);

    Button* but = new Button();
    but->setPosition(Vector2D(400, 450));
    but->setText("    Play");
    but->setName("Play");
    but->setCbFunc(Menu::play);
    widgets.add(but);
  }
  else if (game.getState() == RUN && !Mouse::instance()->isGuiClick() && awaitMapClick_){
    //awaiting click on map for various ingame functions
    float dx = (float)wrld.getMapSize().x/screen.getWidth();
    float dy = (float)wrld.getMapSize().y/screen.getHeight();
    Vector2D click;
    click.x = (int)(x*dx);
    click.y = (int)(y*dy);
    //call function that awaits the click
    awaitMapClick_(click);
    awaitMapClick_ = nullptr;
  }
}

void HQRenderer::mouseMove_(int x, int y, int buttons){
  //the visible area of the virtual screen
  const CGE::VirtualScreen& screen = CGE::Engine::instance()->getVirtualScreen();
  int left = (int)screen.getVisibleMin().x;
  int top = (int)screen.getVisibleMin().y;
  int right = (int)screen.getVisibleMax().x;
  int bottom = (int)screen.getVisibleMax().y;
  
  //if (x == width/2 && y == height/2)
  //  return;
  if(threeD_ && !Mouse::instance()->isMouseActive()){
    float angleY = 0.0f;
    float angleZ = 0.0f;

    int xrel = x - mousePos_.x;
    int yrel = y - mousePos_.y;
    //if (xrel == 0 && yrel == 0)
    //  return;

    //too unprecise to set it every move
    //mousePos_.x = width/2;
    //mousePos_.y = height/2;
    //Mouse::instance()->setMousePos(width/2, height/2);
    bool needsSet = false;
    if (x < left + 10){
      x = right - 10;
      needsSet = true;
    }
    else if (x > right - 10){
      x = left + 10;
      needsSet = true;
    }
    if (y < top + 10){
      y = bottom - 10;
      needsSet = true;
    }
    else if (y > bottom - 10){
      y = top + 10;
      needsSet = true;
    }
    if (needsSet)
      Mouse::instance()->setMousePos(x, y);
    mousePos_.x = x;
    mousePos_.y = y;

    angleY = (float) (-xrel)/400.0f;
    angleZ = (float) (-yrel)/400.0f;

    cam.setLastRotX(cam.getCurrRotX());
    cam.setCurrRotX(cam.getCurrRotX()+angleZ);

    Vector3D look = cam.view();
    Vector3D pos = cam.position();
    Vector3D up = cam.upVector();
    Vector3D axis = (look - pos).cross(up);
    axis = axis.normalized();

    //stop camera from going too high or too low
    if (cam.getCurrRotX() > 1.0){
      cam.setCurrRotX(1.0);
      if (cam.getLastRotX() != 1.0f){
        cam.rotateView(1.0f - cam.getLastRotX(), axis);
      }
    }
    else if (cam.getCurrRotX() < -1.0){
      cam.setCurrRotX(-1.0);
      if (cam.getLastRotX() != -1.0f){
        cam.rotateView(-1.0f - cam.getLastRotX(), axis);
      }
    }
    else{
      cam.rotateView(angleZ, axis);
    }
    cam.rotateView(angleY, Vector3D(0.0, 1.0, 0.0));
  }
  else{
    mousePos_.x = x;
    mousePos_.y = y;
  }
}

void HQRenderer::initialize_(){
  TR_USE(HQ_Renderer);
  //the lit shader with some nice dark fog for the 3D view
  registerHeroquestShaders();
  CGE::Gfx::ShaderDesc desc;
  desc.name = "heroquest/fog";
  if (CGE::Gfx::ShaderLibrary::loadDesc("engine/lit.vert", "heroquest/fog.frag", desc))
    fogShader_ = CGE::Engine::instance()->getRenderDevice()->createShader(desc);
  if (!fogShader_)
    TR_WARN("no fog shader, the 3D view is drawn without fog");

  //init textures
  TextureManager::init();

  //init game
  if (!game.init()){
    EXIT();
    return;
  }

  //start game
  if (!game.start())
    EXIT();

}

void HQRenderer::paint_(){
  game.run();

  //move camera only in 3D
  if (threeD_)
    handleKeys();

  CGE::Engine* engine = CGE::Engine::instance();
  CGE::Gfx::RenderDevice* device = engine->getRenderDevice();
  CGE::Gfx::DrawContext& ctx = *engine->getDrawContext();
  CGE::Forms* forms = engine->getForms();
  const CGE::VirtualScreen& screen = engine->getVirtualScreen();

  //cleared black, the colour of the fog
  CGE::Gfx::PassDesc pass;
  pass.clearColor = {0, 0, 0, 1};
  if (!device->beginPass(pass))
    return;
  ctx.setColor(1.0,1.0,1.0,1.0);

  if (wrld.isLoaded()){
    //number of moves in the upper right corner
    engine->getScreenText(0)->print(1000, 750, std::to_string(game.getMoves()).c_str(), 1);
    //get the nearest vertices and check them for camera collision
    //Vector3D** worldCollision = wrld.getWorld();
    //cam.checkCameraCollision(worldCollision, wrld.getNumberOfVerts());
    //allow lookat changes by mouse
    ctx.setProjection(device->perspective(fieldOV_, aspect_, 0.1f, 100.0f));
    ctx.setView(cam.getViewMatrix());
    //render world without blending, with back face culling
    ctx.setDepth(true, true);
    ctx.setBlend(CGE::Gfx::BlendMode::Opaque);
    ctx.setCull(CGE::Gfx::CullMode::Back);
    if (threeD_){
      ctx.setLitShader(fogShader_.get());
      wrld.render();
      ctx.setLitShader(nullptr);
    }
    else
      wrld.render2D(!plyr.isZargon());
    ctx.setCull(CGE::Gfx::CullMode::Off);
  }
  else{
    setViewTo3D(false);
  }

  //the 2D screens and overlays in virtual coordinates (y up), added to what is there
  ctx.setProjection(screen.ortho(*device));
  ctx.setView(CGE::Matrix(CGE::Matrix::Identity));
  ctx.setDepth(false, false);
  ctx.setBlend(CGE::Gfx::BlendMode::Additive);
  ctx.setColor(1.0,1.0,1.0,1.0);

  if (!wrld.isLoaded()){
    ctx.setTexture(TextureManager::instance()->otherTex[0].get());
    CGE::MatrixStack& model = ctx.model();
    model.push();
    model.translate(screen.getWidth()/2.0f, screen.getHeight()/2.0f, 0);
    model.scale((float)screen.getWidth(), (float)screen.getHeight(), 1);
    forms->drawQuad(ctx);
    model.pop();
    ctx.setTexture(nullptr);
  }

  //render inventory
  if (inventory_){
    inventory_->render();
    inventory_->update();
  }

  if (trade_){
    plyr.getTrade()->render();
    plyr.getTrade()->update();
  }

  CGE::ScreenText *f = engine->getScreenText(0);
  f->setColor(0,1,0);
  f->print(20, 720, ("Current Frames Per Second: "+std::to_string(engine->getFPS())).c_str(), 0);

  //interpolate hero positions
  for (int i = 0; i < wrld.getHeroSize(); i++){
    if (wrld.getStarts()[i] != Vector2D(-1,-1))
      continue;
    wrld.getHeros()[i].update();
  }

  //interpolate hero positions
  if (wrld.isLoaded()){
    for (unsigned i = 0; i < wrld.getMonsters().size(); i++){
      if (wrld.getMonsters()[i]->getStatus())
        wrld.getMonsters()[i]->update();
    }
  }

  //the buttons, input fields and message boxes
  widgets.render();

  ctx.setBlend(CGE::Gfx::BlendMode::Opaque);
  device->endPass();
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