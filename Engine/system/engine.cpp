#include "engine.h"
#include "../renderer/OpenGL/OGLrenderer.h"
#include "../renderer/DirectX/DXrenderer.h"
#include "../renderer/GLES/GLESrenderer.h"
#include "../renderer/dummyrenderer.h"
#include "../input/mouse.h"
#include "../input/keyboard.h"
#include "../window/nativeWindows.h"
#include "script.h"
#include "../gui/console.h"
#include "../renderer/forms.h"
#include "../physics/Simulator.h"

#ifdef WIN32
#define TIME_FACTOR 0.001
#endif
#ifdef UNIX
#define TIME_FACTOR 0.01
#endif

void (*internalEngineMain)(int argc, char** argv) = NULL;

ofstream System::Log("engine.log");

System::Engine* System::Engine::eng = NULL;

System::Engine::Engine(){
  win_ = NULL;
  rend_ = NULL;
  isUp_ = false;
  frameInterval_ = 0;
  frameTime_ = 0;
  frameInterval_ = 0;
  lastTime_ = 0;
  fpscounter_ = 0;
  fps_ = 0;
  input_ = NULL;
  console_ = NULL;
  clear_ = false;
  guitex_ = Mutex();
  mSimulator = NULL;
  Log << "Engine instance created\n";
}

void System::Engine::init(){
  eng = new Engine();
}

void System::Engine::startup(int argc, char** argv){
  Script::init();
  Script::instance()->initEnv();
  string type = Script::instance()->getStringSetting("renderer");
  maxFramerate_ = ::System::Script::instance()->getNumberSetting("timeScheme");
  graphics_ = Script::instance()->getBoolSetting("graphics");
  physics_ = Script::instance()->getBoolSetting("physics");

  if (graphics_){
    if (type == "OpenGL"){
#ifdef OPENGL
        rend_ = new ::Graphics::OGLRenderer();
#else
        EXIT2("OpenGL support is not compiled in\n");
#endif
    }
    else if (type == "DirectX"){
#ifdef DIRECTX
        rend_ = new ::Graphics::DXRenderer();
#else
        EXIT2("DirectX support is not compiled in\n");
#endif
    }
    else if (type == "GLES"){
#ifdef GLES
      rend_ = new ::Graphics::GLESRenderer();
#else
      EXIT2("OpenGL ES support is not compiled in\n");
#endif
    }
    else{
      EXIT2("No valid renderer specified in engine.ini");
    }

#ifdef WIN32
    win_ = new ::Windows::WindowsWindow(rend_);
#endif
#ifdef UNIX
    win_ = new ::Windows::X11Window(rend_);
#endif
  }
  else{
    rend_ = new ::Graphics::DummyRenderer();
  }
  if (graphics_)
    win_->init("Captains Game Engine");
  Input::Keyboard::init();
  Input::Mouse::init(graphics_);

  if (physics_)
    mSimulator = new CGE::Simulator();
  
  if (internalEngineMain != NULL)
    internalEngineMain(argc, argv);
  
  fnt_ = new ::Graphics::Font*[3];
  for (int i = 0; i < 3; i++){
    fnt_[i] = new ::Graphics::Font();
  }
  if (graphics_){
    fnt_[0]->buildFont();
    fnt_[1]->setVB(fnt_[0]->getVB());
    fnt_[2]->setVB(fnt_[0]->getVB());
    forms_ = new ::Graphics::Forms();
  }
  console_ = new ::Gui::Console();
  isUp_ = true;
  rend_->initRendering();
  //isUp_ = true;
}

void System::Engine::shutdown(){
  if (!isUp_)
    return;
  Log << "engine shutting down\n";
  isUp_ = false;
  SAFE_DELETE(mSimulator);
  Input::Keyboard::release();
  Input::Mouse::release();
  SAFE_DELETE(console_);
  fnt_[0]->killFont();
  for (int i = 0; i < 3; i++){
    if (i != 0)
      fnt_[i]->setVB(NULL);
    SAFE_DELETE(fnt_[i]);
  }
  SAFE_DELETE_ARRAY(fnt_);
  SAFE_DELETE(forms_);
  if (win_)
    win_->kill();
  SAFE_DELETE(win_);
  SAFE_DELETE(rend_);
  Script::kill();
  Log.close();
  //SAFE_DELETE(eng);
  exit(0);
}

void System::Engine::run(){
  //handle physics, KI, ...
  if (physics_){
    mSimulator->doSimulationStep(frameInterval_);
  }

  console_->update();
  Input::Keyboard::instance()->processPressedKeys((float)frameInterval_);

  //something with the GUI elements changed, so update here
  for (unsigned i = 0; i < remBut_.size(); i++){
    removeGuiListener(remBut_[i]);
  }
  remBut_.clear();
  if (newBut_.size() != 0 || clear_){
    clear_ = false;
    clearListeners();
    guiElems_ = newBut_;
    newBut_.clear();
  }

  if (graphics_){
    //render scene
    rend_->renderScene();

    rend_->resetModelView();
    rend_->ortho(1024, 768);
    rend_->translate(-512, -384, 0);
    rend_->blendFunc(Graphics::BLEND_SRC_ALPHA, Graphics::BLEND_ONE);
    rend_->enableBlend(true);
  
    //render GUI-elements
    guitex_.lock();

    fnt_[0]->render();
  
    list< ::Gui::GuiElement*>::iterator iter2;
    for (iter2 = guiElems_.begin(); iter2 != guiElems_.end(); iter2++){
      (*iter2)->render();
    }

    fnt_[1]->render();

    System::Engine::instance()->getConsole()->render();

    fnt_[2]->render();

    guitex_.unlock();
  
    rend_->enableBlend(false);

    rend_->swapBuffers();
  }

  //calculate frame rate
  double currentTime;
  if (maxFramerate_ == 0){
    currentTime = GetTickCount()*TIME_FACTOR;
    //cerr << currentTime;
    frameInterval_ = currentTime-frameTime_;
    frameTime_ = currentTime;
  }
  else{
    while(((currentTime = GetTickCount()*TIME_FACTOR)-frameTime_) <= 1.0/maxFramerate_);
    frameInterval_ = currentTime-frameTime_;
    frameTime_ = currentTime;
  }

  fpscounter_++;

  //one second passed by
  if (currentTime - lastTime_ > 1.0){
    lastTime_ = currentTime;
    fps_ = fpscounter_;
    fpscounter_ = 0;
  }
}
 

// set the current input field that should be active
void System::Engine::setActiveInput(::Gui::InputField* field){
  if (field == NULL){
    input_ = NULL;
    return;
  }
  if (input_ != NULL){
    input_->removeChar();
  }
  input_ = field;
  input_->addChar('_');
}

// clears the input and button listeners
void System::Engine::clearListeners(bool immediate){
  if (!immediate){
    clear_ = true;
    return;
  }
  guitex_.lock();
  list< ::Gui::GuiElement* >::iterator iter2;
  for (iter2 = guiElems_.begin(); iter2 != guiElems_.end(); iter2++){
    delete (*iter2);
  }
  guiElems_.clear();
  fnt_[0]->clear();
  fnt_[1]->clear();
  fnt_[2]->clear();
  guitex_.unlock();
}


// remove the input listener at position idx
/*
void System::Engine::removeInputListener(int idx){
  guitex_.lock();
  list< ::Gui::InputField*>::iterator iter = listeners_.begin();
  for (int i = 0; i < idx; i++){
    iter++;
  }
  listeners_.erase(iter);
  guitex_.unlock();
}*/

// remove the button listener at position idx
void System::Engine::removeGuiListener(int idx, bool immediate){
  if (!immediate){
    remBut_.push_back(idx);
    return;
  }
  guitex_.lock();
  list< ::Gui::GuiElement*>::iterator iter = guiElems_.begin();
  for (int i = 0; i < idx; i++){
    iter++;
  }
  delete *iter;
  guiElems_.erase(iter);
  guitex_.unlock();
}


// remove the button listener with a certain name
void System::Engine::removeGuiListener(const string& name){
  guitex_.lock();
  list< ::Gui::GuiElement*>::iterator iter = guiElems_.begin();
  for (unsigned i = 0; i < guiElems_.size(); i++){
    if((*iter)->getName() == name){
      delete *iter;
      guiElems_.erase(iter);
      break;
    }
    iter++;
  }
  guitex_.unlock();
}


// remove all button listeners beginning at idx
void System::Engine::removeGuiListeners(int idx){
  guitex_.lock();
  list< ::Gui::GuiElement*>::iterator iter = guiElems_.begin();
  for (int i = 0; i < idx; i++){
    iter++;
  }
  while (iter != guiElems_.end()){
    delete *iter;
    iter = guiElems_.erase(iter);
  }
  guitex_.unlock();
}

// get a button by name
::Gui::GuiElement* System::Engine::getGuiListener(const string& name){
  list< ::Gui::GuiElement*>::iterator iter = guiElems_.begin();
  for (unsigned i = 0; i < guiElems_.size(); i++){
    if ((*iter)->getName() == name){
      return *iter;
    }
    iter++;
  }
  return NULL;
}
