#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <renderer/GL2/GL2Shader.h>
#include <GL/gl.h>

#include "renderer/renderer.h"
#include "renderer/font.h"
#include "renderer/forms.h"
#include "gui/gui.h"
#include "input/mouse.h"
#include "input/keyboard.h"
#include "window/window.h"

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

#define line *CGE::Engine::instance()->getFont(0)

using Gui::InputField;
using Gui::Button;
using Input::Mouse;

HQRenderer* HQRenderer::rend_ = NULL;

HQRenderer::HQRenderer(CGE::Renderer* rend){
  render_ = rend;
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
  awaitMapClick_ = NULL;
  mousePos_ = Vector2D(0,0);

  inventory_ = NULL;
  trade_ = false;
  m3DShader = NULL;
}

HQRenderer::~HQRenderer(){
  delete m3DShader;
}

void HQRenderer::resize_(int width, int height){
  aspect_ = (float)width/height;
  render_->projection(fieldOV_, aspect_, 0.1f, 100.0f);
}

//special input
void HQRenderer::special_(int key){
  switch (key){
  case KEY_ESCAPE:
    if (inventory_){
      Menu::close(NULL);
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
    msg.send(toStr(CREAT_CHANGE)+" "+toStr(pos.x)+" "+toStr(pos.y));
    break;
  }
  case 'i':
    if (game.getState() == RUN)
      Menu::inventory(NULL);
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
	    bool selfHero = dynamic_cast<Hero*>(wrld.getObject(pos)) == NULL ? false : true;
      bool objectHero = true;
	    bool objectMonster = true;
      bool objectFurni = false;
      bool overlayNotWalkable = false;
	    
      GameObject *o = wrld.getObject(moveTo);
	    if (o){
		    objectHero = dynamic_cast<Hero*>(o) == NULL ? false : true;
		    objectMonster = dynamic_cast<Monster*>(o) == NULL ? false : true;
        objectFurni = dynamic_cast<Furniture*>(o) == NULL ? false : true;
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
  if (game.getState() == PREPARE && !Mouse::instance()->isGuiClick() && clickedField_ == Vector2D(-1,-1) && !trade_){
    float dx = (float)wrld.getMapSize().x/SCREENWIDTH;
    float dy = (float)wrld.getMapSize().y/SCREENHEIGHT;
    clickedField_.x = (int)(x*dx);
    clickedField_.y = (int)(y*dy);

    //buttons to add a hero at this position
    CGE::Engine::instance()->getFont(0)->setColor(1,1,1);
    CGE::Engine::instance()->getFont(0)->glPrint(250, 500, "Hero name:", 1, (float)HUGE_VAL);
    InputField* in = new InputField();
    in->setName("Playname");
    in->setPosition(Vector2D(400, 500));
    CGE::Engine::instance()->addGuiListener(in);

    Button* but = new Button();
    but->setPosition(Vector2D(400, 450));
    but->setText("    Play");
    but->setName("Play");
    but->setCbFunc(Menu::play);
    CGE::Engine::instance()->addGuiListener(but);
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

void HQRenderer::mouseMove_(int x, int y, int buttons){
  int width = CGE::Engine::instance()->getWindow()->getWidth();
  int height = CGE::Engine::instance()->getWindow()->getHeight();
  
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
    if (x < 10){
      x = width - 10;
      needsSet = true;
    }
    else if (x > width - 10){
      x = 10;
      needsSet = true;
    }
    if (y < 10){
      y = height - 10;
      needsSet = true;
    }
    else if (y > height - 10){
      y = 10;
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

char const * vs_src =
"attribute vec3 pos;\n"
"attribute vec4 color;\n"
"attribute vec2 texcoord;\n"
"attribute vec3 normal;\n"
"\n"
"uniform mat4 mvp;\n"
"\n"
"varying vec2 uvcoord;\n"
"varying vec4 vcolor;\n"
"\n"
"void main(){\n"
"  uvcoord = texcoord;\n"
"  vcolor = color;\n"
"  gl_Position = mvp*vec4(pos, 1.0);\n"
"};\n"
"";

char const * fs_src =
"uniform sampler2D texture;\n"
"uniform bool textureEnabled;\n"
"\n"
"varying vec2 uvcoord;\n"
"varying vec4 vcolor;\n"
"\n"
"void main(){\n"
"  vec4 color = vec4(1.0);\n"
"  if (textureEnabled)\n"
"     color = texture2D(texture, uvcoord);\n"
"  gl_FragColor = color*vcolor;\n"
"};\n"
"";

void HQRenderer::initialize_(){
  //init shader
  /*if (CGE::Engine::instance()->getRenderer()->getRenderType() == CGE::OpenGL2){
    m3DShader = new CGE::GL2Shader();
    m3DShader->addShader(GL_VERTEX_SHADER, vs_src);
    m3DShader->addShader(GL_FRAGMENT_SHADER, fs_src);
    m3DShader->bindAttribLocation(0, "pos");
    m3DShader->bindAttribLocation(1, "color");
    m3DShader->bindAttribLocation(2, "texcoord");
    m3DShader->bindAttribLocation(3, "normal");
    m3DShader->linkShaders();
    m3DShader->activate();
    int tex = m3DShader->getUniformLocation("texture");
    m3DShader->uniform(tex, 0);//texture (uniform 32) at stage 0
  }*/
  //init textures
  TextureManager::init();

  render_->setClearColor(Vector3D());

  //init game
  if (!game.init()){
    EXIT();
    return;
  }

  //setup some nice dark fog
  float fogColor[4] = {0, 0, 0, 1};
  glFogi(GL_FOG_MODE, GL_EXP2);
  glFogfv(GL_FOG_COLOR, fogColor);
  glFogf(GL_FOG_DENSITY, 0.035f);
  glHint(GL_FOG_HINT, GL_DONT_CARE);
  glFogf(GL_FOG_START, 0.0);
  glFogf(GL_FOG_END, 100.0);
  glEnable(GL_FOG);

  //back face culling
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);

  //start game
  if (!game.start())
    EXIT();

}

void HQRenderer::paint_(){
  game.run();

  //move camera only in 3D
  if (threeD_)
    handleKeys();

  render_->projection(fieldOV_, aspect_, 0.1f, 100.0f);
  
  render_->clear(ZBUFFER | COLORBUFFER);
  render_->setColor(1.0,1.0,1.0,1.0);
  render_->resetModelView();

  if (wrld.isLoaded()){
    //number of moves in the upper right corner
    CGE::Engine::instance()->getFont(0)->print(1000, 750, toStr(game.getMoves()).c_str(), 1);
    //get the nearest vertices and check them for camera collision
    //Vector3D** worldCollision = wrld.getWorld();
    //cam.checkCameraCollision(worldCollision, wrld.getNumberOfVerts());
    //allow lookat changes by mouse
    cam.look();
    glEnable(GL_DEPTH_TEST);
    //render world without blending
    glDisable(GL_BLEND);
    //glEnable(GL_TEXTURE_2D);
    if (threeD_)
      wrld.render();
    else
      wrld.render2D(!plyr.isZargon());
    glEnable(GL_BLEND);
  }
  else{
    setViewTo3D(false);
  }
  
  render_->ortho(SCREENWIDTH, SCREENHEIGHT);
  render_->translate(-SCREENWIDTH/2, -SCREENHEIGHT/2, 0);
  render_->blendFunc(CGE::BLEND_SRC_ALPHA, CGE::BLEND_ONE);
  render_->enableBlend(true);
  render_->enableTexturing(true);
  
  if (!wrld.isLoaded()){
    TextureManager::instance()->otherTex[0]->activate();
    CGE::Forms* form = CGE::Engine::instance()->getForms();
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
  render_->resetModelView();
  render_->translate(-512, -384, 0);
  if (inventory_){
    inventory_->render();
    inventory_->update();
  }
  
  if (trade_){
    plyr.getTrade()->render();
    plyr.getTrade()->update();
  }
  
  CGE::Font *f = CGE::Engine::instance()->getFont(0);
  f->setColor(0,1,0);
  f->print(20, 720, ("Current Frames Per Second: "+toStr(CGE::Engine::instance()->getFPS())).c_str(), 0);
  
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
