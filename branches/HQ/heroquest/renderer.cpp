//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: renderer.qt.cpp                                             |
//  ==================================================================

#include <math.h>
#ifndef WIN32
#include <sys/time.h>
#endif
#include <time.h>
#include <iostream>
#include <fstream>
#include <string>

#include "vector.hh"
#include "texture.hh"
#include "camera.hh"
#include "world.hh"
#include "font.hh"
#include "gui.hh"
#include "console.hh"
#include "message.hh"
#include "opcodes.hh"
#include "player.hh"
#include "gamestate.hh"
#include "math.hh"
#include "renderer.hh"


using std::ifstream;

extern string path;

//init some values
Renderer::Renderer() : buttonPressed_(false){
  //init fps counters
  framesPerSecond_ = 0;
  frameTime_ = 0;
  lastTime_ = 0;

  //camera
  aspect_ = 1.0;
  fieldOV_ = 40;

  // the Renderer should not relay keybord input
  input_ = NULL;

  // start with 2D view
  threeD_ = false;
  // and mouse pointer there
  mousePointer_ = true;

  //misc settings
  clear_ = false;
  cf_ = Vector2D(-1,-1);
  ask_ = false;
  quit_ = false;
  ignoreMouse_ = false;
  awaitMapClick_ = NULL;

  // inventory to be rendered
  inventory_ = NULL;
}

Renderer::Renderer(const Renderer& r){
  input_ = r.input_;
  listeners_ = list<InputField*>(r.listeners_);
  buttons_ = list<Button*>(r.buttons_);
  clear_ = r.clear_;
  newIn_ = list<InputField*>(r.newIn_);
  newBut_ = list<Button*>(r.newBut_);
  width_ = r.width_;
  height_ = r.height_;
  aspect_ = r.aspect_;
  fieldOV_ = r.fieldOV_;
  threeD_ = r.threeD_;
  quit_ = r.quit_;
  frameInterval_ = r.frameInterval_;
  framesPerSecond_ = r.framesPerSecond_;
  lastTime_ = r.lastTime_;
  frameTime_ = r.frameTime_;
  fps_ = r.fps_;
  mousePointer_ = r.mousePointer_;
  mousex_ = r.mousex_;
  mousey_ = r.mousey_;
  cf_ = r.cf_;
  awaitMapClick_ = r.awaitMapClick_;
  inventory_ = r.inventory_;
}

// Destructor
Renderer::~Renderer()
{
  text.killFont();
}

//OpenGL init
void Renderer::initializeGL()
{
  //Load textures
  if (!tex.loadGLTextures()){
    return;
  }
  
  glEnable(GL_TEXTURE_2D);

  //build font
  text.buildFont();
  output.setBase(text.getBase());
  line.setBase(text.getBase());
  text.toggle();

  //back face culling
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);
  
  //interpolate vertex colors
  glShadeModel(GL_SMOOTH);
  
  glClearColor( 0.0, 0.0, 0.0, 1.0 );

  //setup some nice dark fog
  float fogColor[4] = {0.0, 0.0, 0.0, 1.0};
  glFogi(GL_FOG_MODE, GL_EXP2);
  glFogfv(GL_FOG_COLOR, fogColor);
  glFogf(GL_FOG_DENSITY, 0.035f);
  glHint(GL_FOG_HINT, GL_DONT_CARE);
  glFogf(GL_FOG_START, 0.0);
  glFogf(GL_FOG_END, 100.0);
  glEnable(GL_FOG);
  
  //misc settings (depth test, blending, vertex arrays)
  glEnable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  
}

//viewport changed
void Renderer::resizeGL( int w, int h )
{
  aspect_ = (GLfloat)w / (GLfloat)h;
  width_ = w;
  height_ = h;

  glViewport( 0, 0, (GLint)w, (GLint)h );

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  gluPerspective(fieldOV_, aspect_, 0.1f, 100.0f);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

//main render function
void Renderer::paintGL()
{
  glColor3f(1.0, 1.0, 1.0);
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glLoadIdentity();
  
  if (wrld.isLoaded()){
    //number of moves in the upper right corner
    output.glPrint(1000, 750, toStr(game.getMoves()).c_str(), 1,0);
    //get the nearest vertices and check them for camera collision
    Vector3D **worldCollision = wrld.getWorld();
    cam.checkCameraCollision(worldCollision, wrld.getNumberOfVerts());
    //allow lookat changes by mouse
    cam.look();
    //render world without blending
    glDisable(GL_BLEND);
    if (threeD_){
      wrld.render();
      //quick and dirty frame limitation
      //FIND BETTER SOLUTION
      //wait();
      //wait();
      //wait();
    }
    else{
      //zargon sees everything
      wrld.render2D(!plyr.isZargon());
    }
    glEnable(GL_BLEND);
  }
  //else{
  //  wait();
  //  wait();
  //  wait();
  //  wait();
  //}
  
 
  //change to orthographic view
  ortho();
  
  if (!wrld.isLoaded()){
    //show background image
    glBindTexture(GL_TEXTURE_2D, tex.otherTex[0]);
    glBegin(GL_QUADS);
      glTexCoord2f(1, 0);
      glVertex2i(SCREENWIDTH, SCREENHEIGHT);
      glTexCoord2f(0, 0);
      glVertex2i(0, SCREENHEIGHT);
      glTexCoord2f(0, 1);
      glVertex2i(0, 0);
      glTexCoord2f(1, 1);
      glVertex2i(SCREENWIDTH, 0);
    glEnd();
  }

  //render inventory
  if (inventory_){
    inventory_->render();
    inventory_->update();
  }

  //something with the GUI elements changed, so update here
  if (newIn_.size() != 0 || newBut_.size() != 0 || clear_){
    clear_ = false;
    clearListeners();
    listeners_ = newIn_;
    buttons_ = newBut_;
    newIn_.clear();
    newBut_.clear();
  }
 
  //render GUI-elements
  list<InputField*>::iterator iter;
  for (iter = listeners_.begin(); iter != listeners_.end(); iter++){
    (*iter)->render();
  }
  
  list<Button*>::iterator iter2;
  for (iter2 = buttons_.begin(); iter2 != buttons_.end(); iter2++){
    (*iter2)->render();
  }
  
  //render texts
  text.render();
  output.render();
  line.render();

  //render console
  consol.render();
  
  //switch back to perspective
  back();

  //interpolate hero positions
  for (int i = 0; i < wrld.getHeroSize(); i++){
    if (wrld.getStarts()[i] != Vector2D(-1,-1))
      continue;
    wrld.getHeros()[i].update();
  }

  //interpolate monster positions
  if (wrld.isLoaded()){
    for (unsigned i = 0; i < wrld.getMonsters().size(); i++){
      if (wrld.getMonsters()[i]->getStatus())
        wrld.getMonsters()[i]->update();
    }
  }
  
}

//change to orthographic view
void Renderer::ortho(){
  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glEnable(GL_TEXTURE_2D);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, SCREENWIDTH, 0, SCREENHEIGHT, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
}

//switch back to perspective view
void Renderer::back(){
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glPopAttrib();
}


// mouse button pressed
void Renderer::mousePressEvent(SDL_MouseButtonEvent * m){
  if (buttonPressed_)
    return;
  buttonPressed_ = true;
  mousex_ = m->x;
  mousey_ = m->y;

  //map to virtual resolution
  mousex_ = (int)((float)mousex_/width_*SCREENWIDTH);
  mousey_ = (int)((float)mousey_/height_*SCREENHEIGHT);

  bool gui_click = false;
  
  // click on a GUI-element?
  list<InputField*>::iterator iter;
  for (iter = listeners_.begin(); iter != listeners_.end(); iter++){
    if ((*iter)->isHit(Vector2D(mousex_,SCREENHEIGHT-mousey_))){
      //set only input focus if console is not active
      if (!consol.isActive()){
	      //another input field was active, so remove cursor
        if (input_ != NULL){
          input_->removeChar();
        }
        input_ = (*iter);
        input_->addChar('_');
	      gui_click = true;
        break;
      }
    }
  }

  list<Button*>::iterator iter2;
  for (iter2 = buttons_.begin(); iter2 != buttons_.end(); iter2++){
    if ((*iter2)->isClicked(Vector2D(mousex_,SCREENHEIGHT-mousey_))){
      //allow only clicks, if the console is deactivated
      if (!consol.isActive()){
        if (input_ != NULL){
          input_->removeChar();
        }
        input_ = NULL;
        (*iter2)->process();
        gui_click = true;
        break;
      }
    }
  }

  //clickable map
  if (game.getState() == PREPARE && !gui_click && cf_ == Vector2D(-1,-1)){
    float dx = (float)wrld.getMapSize().x/SCREENWIDTH;
    float dy = (float)wrld.getMapSize().y/SCREENHEIGHT;
    cf_.x = (int)(mousex_*dx);
    cf_.y = (int)(mousey_*dy);
    
    //buttons to add a hero at this position
    output.glPrint(250, 500, "Hero name:", 1, (float)HUGE_VAL);
    InputField* in = new InputField();
    in->setPosition(Vector2D(400, 500));
    addInputListener(in);
    
    Button* but = new Button();
    but->setPosition(Vector2D(400, 450));
    but->setText("    Play");
    void (Renderer::*p)();
    p = &Renderer::play;
    but->setCbFunc(p);
    addButtonListener(but);
  }
  else if (game.getState() == RUN && !gui_click && awaitMapClick_){
    //awaiting click on map for various ingame functions
    float dx = (float)wrld.getMapSize().x/SCREENWIDTH;
    float dy = (float)wrld.getMapSize().y/SCREENHEIGHT;
    Vector2D click;
    click.x = (int)(mousex_*dx);
    click.y = (int)(mousey_*dy);
    //call function that awaits the click
    (gl->*awaitMapClick_)(click);
    awaitMapClick_ = NULL;
  }
}


//release mouse button
void Renderer::mouseReleaseEvent(SDL_MouseButtonEvent * /* m */ ){
  buttonPressed_ = false;
}


//when the mouse is moved
void Renderer::mouseMoveEvent(SDL_MouseMotionEvent * m){
  // the mouse changes the camera view
  if (ignoreMouse_){
    ignoreMouse_ = false;
    return;
  }
  if (m->x == width_/2 && m->y == height_/2)
    return;
  if(threeD_ && !mousePointer_){
    float angleY = 0.0f;
    float angleZ = 0.0f;
 
    if (m->xrel == 0 && m->yrel == 0) return;
 
    angleY = (float) (-m->xrel)/400.0f;
    angleZ = (float) (-m->yrel)/400.0f;
 
    cam.setLastRotX(cam.getCurrRotX());
    cam.setCurrRotX(cam.getCurrRotX()+angleZ);
    
    Vector3D look = cam.view();
    Vector3D pos = cam.position();
    Vector3D up = cam.upVector();
    Vector3D axis = (look - pos).cross(up);
    axis = axis.normalized();

    //stop camera from going too high or too low
    if (cam.getCurrRotX() > 1.0f){
      cam.setCurrRotX(1.0);
      if (cam.getLastRotX() != 1.0f){
        cam.rotateView(1.0f - cam.getLastRotX(), axis);
      }
    }
    else if (cam.getCurrRotX() < -1.0){
      cam.setCurrRotX(-1.0);
      if (cam.getLastRotX() != -1.0f){
        cam.rotateView(-1.0f-cam.getLastRotX(), axis);
      }
    }
    else{
      cam.rotateView(angleZ, axis);
    }
    
    cam.rotateView(angleY, Vector3D(0.0,1.0,0.0));
  }
}

void Renderer::setEvent(){
  SDL_Event event;
  SDL_UserEvent userevent;

  userevent.type = SDL_USEREVENT;
  userevent.code = 0;
  userevent.data1 = NULL;
  userevent.data2 = NULL;

  event.type = SDL_USEREVENT;
  event.user = userevent;

  SDL_PushEvent(&event);
}

//calculates frame rate
void Renderer::calculateFrameRate(){
  char strFrameRate[70] = {0};

  //in seconds
  float currentTime = SDL_GetTicks()*0.001;
  frameInterval_ = currentTime - frameTime_;
  frameTime_ = currentTime;

  framesPerSecond_++;

  //one second has passed by
  if (currentTime - lastTime_ > 1.0){
    lastTime_ = currentTime;
    fps_ = (short)framesPerSecond_;

    sprintf(strFrameRate, "Current Frames Per Second: %d", int(framesPerSecond_));

    text.setColor(0.0,1.0,0.0);
    text.glPrint(20, 720, strFrameRate, 0, 1);

    framesPerSecond_ = 0;
  }
}

//if a key is pressed
void Renderer::keyPressEvent(SDL_keysym *key){
  //input goes to input field
  if (input_ != NULL){
    //input finished
    if (key->sym == SDLK_RETURN){
      input_->removeChar();
      input_->end();
      //Normal Input ==> single line
      if (!consol.isActive())
        input_ = NULL;
      return;
    }
    //delete one char
    else if (key->sym == SDLK_BACKSPACE){
      input_->removeChar();
      input_->removeChar();
      input_->addChar('_');
      return;
    }
    //history
    else if (key->sym == SDLK_UP){
      consol.Up();
    }
    else if (key->sym == SDLK_DOWN){
      consol.Down();
    }
    //a normal character to display
    else{
      char c = (char)key->unicode;
      if (c > 31 && c < 127){
	      input_->removeChar();
        input_->addChar(c);
	      input_->addChar('_');
	      return;
      }
    }
  }
  // key handling, if no input field is active
  switch (key->sym){
  case SDLK_ESCAPE:
    //quit console
    if (consol.isActive()){
      consol.toggle();
      input_->clear();
      input_ = NULL;
      break;
    }
    line << "Do yo really want to quit (y/n)";
    ask_ = true;
    break;
  case SDLK_y:
    //quit application
    if (ask_)
      msg.quit();
    break;
  case SDLK_n:
    ask_ = false;
    line << " ";
    break;
  //lock /unlock mouse pointer in 3D
  case SDLK_f:
    if (threeD_){
      mousePointer_ = !mousePointer_;
      if (mousePointer_)
        SDL_ShowCursor(1);
      else{
#ifdef WIN32
	setEvent();
	SDL_WarpMouse(width_/2,height_/2);
  SDL_PumpEvents();
	wait();
#endif
        SDL_ShowCursor(0);
      }
    }
    break;
  //switch between 2D / 3D
  case SDLK_q:
  case SDLK_m:
    if (game.getState() != RUN)
      break;
    threeD_ = !threeD_;
    if (threeD_ && wrld.isLoaded()){
      mousePointer_ = false;
#ifdef WIN32
			setEvent();
      SDL_WarpMouse(width_/2,height_/2);
      SDL_PumpEvents();
      wait();
#endif
			SDL_ShowCursor(0);
    }
    else
      SDL_ShowCursor(1);
    break;
  //open console
  case SDLK_TAB:
    consol.toggle();
    if (consol.isActive()){
      input_ = consol.GetInputField();
      input_->addChar('_');
    }
    else{
      input_->clear();
      input_ = NULL;
    }
    break;
  //switch debug output
  case SDLK_F1:
    text.toggle();
    break;
  case SDLK_F2:
    if (SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_ON)
      SDL_WM_GrabInput(SDL_GRAB_OFF);
    else
      SDL_WM_GrabInput(SDL_GRAB_ON);
    SDL_WM_ToggleFullScreen(SDL_GetVideoSurface());
    break;
  case SDLK_w:
  case SDLK_s:
  case SDLK_a:
  case SDLK_d:
    keys_[key->sym] = true;
    break;
  default:
    //keys_[key->sym] = true;
    break;
  }
}

//if key is released
void Renderer::keyReleaseEvent(SDL_keysym *key){
  switch (key->sym){
    case SDLK_w:
    case SDLK_s:
    case SDLK_a:
    case SDLK_d:
	keys_[key->sym] = false;
      break;
    default:
      break;
  }
}

// handle keys continously
void Renderer::handleKeys(){
  float speed = cam.getSpeed()*gl->getFrameInterval();
  if (keys_[SDLK_w])
    cam.moveCamera(speed);
  if (keys_[SDLK_s])
    cam.moveCamera(-speed);
  if (keys_[SDLK_a])
    cam.strafeCamera(-speed);
  if (keys_[SDLK_d])
    cam.strafeCamera(speed);
}
	
// quit application
void Renderer::quit(){
  quit_ = true;
}

void Renderer::removeInputListener(int idx){
  list<InputField*>::iterator iter = listeners_.begin();
  for (int i = 0; i < idx; i++){
    iter++;
  }
  listeners_.erase(iter);
}

void Renderer::removeButtonListener(int idx){
  list<Button*>::iterator iter = buttons_.begin();
  for (int i = 0; i < idx; i++){
    iter++;
  }
  delete *iter;
  buttons_.erase(iter);
}


// remove all button listeners beginning at idx
void Renderer::removeButtonListeners(int idx){
  list<Button*>::iterator iter = buttons_.begin();
  for (int i = 0; i < idx; i++){
    iter++;
  }
  while (iter != buttons_.end()){
    delete *iter;
    iter = buttons_.erase(iter);
  }
}

// clear all listeners
void Renderer::clearListeners(bool immediate){
  if (!immediate){
    clear_ = true;
    return;
  }
  list<InputField*>::iterator iter;
  for (iter = listeners_.begin(); iter != listeners_.end();iter++){
    delete (*iter);
  }
  list<Button*>::iterator iter2;
  for (iter2 = buttons_.begin(); iter2 != buttons_.end();iter2++){
    delete (*iter2);
  }
  listeners_.clear();
  buttons_.clear();
  output.clear();
}



void Renderer::setViewTo3D(bool threeD){
  threeD_ = threeD;
  if (threeD_ && wrld.isLoaded()){
      mousePointer_ = false;
#ifdef WIN32
			setEvent();
      SDL_WarpMouse(width_/2,height_/2);
      SDL_PumpEvents();
      wait();
#endif
			SDL_ShowCursor(0);
    }
    else
      SDL_ShowCursor(1);
}


