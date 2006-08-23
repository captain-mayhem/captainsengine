#include "../renderer/OGLrenderer.h"
#include "../renderer/DXrenderer.h"
#include "../renderer/dummyrenderer.h"
#include "../input/mouse.h"
#include "engine.h"

extern void engineMain(int argc, char** argv);

#ifdef WIN32

#include <windows.h>

#define TIME_FACTOR 0.001

int WINAPI WinMain(HINSTANCE instance, HINSTANCE oldinstance, LPSTR cmdline, int cmdShow){
  MSG msg;
  char* argv[2];
  argv[0] = "Hero-Engine";
  argv[1] = cmdline;
  System::Engine::init();
  System::Engine::instance()->startup(2, argv);
  //Enter gameloop
  while(true){
    while (PeekMessage(&msg,NULL,0,0,PM_REMOVE)){
      if(msg.message == WM_QUIT)
        break;
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    //Do not use WM_MouseMove-Event, because it causes the cursor to freeze
    POINT p;
    GetCursorPos(&p);
    Input::Mouse::instance()->move(p.x, p.y, 0);
    System::Engine::instance()->run();
    if (System::Engine::instance()->getRenderer()->getRenderType() == Graphics::OpenGL){
      SwapBuffers(dynamic_cast<Graphics::OGLRenderer*>(System::Engine::instance()->getRenderer())->getDevice());
    }
  }
  System::Engine::instance()->shutdown();
  return (int)msg.wParam;
}
#endif

#include "../window/nativeLinux.h"
#include "../input/keyboard.h"
#include "../renderer/font.h"
#include "../renderer/forms.h"
#include "../gui/gui.h"

#ifdef UNIX
#include <X11/Xlib.h>
#include <GL/glx.h>

#define TIME_FACTOR 0.01

int main(int argc, char** argv){
  System::Engine::init();
  System::Engine::instance()->startup(argc, argv);
  XEvent event;
  bool running = true;
  Windows::X11Window* win = dynamic_cast<Windows::X11Window*>(System::Engine::instance()->getWindow());
  Graphics::Renderer* rend = System::Engine::instance()->getRenderer();
  Display* disp = win->getDisplay();
  XSynchronize(disp, true);
  
  while(running){
    int x = XPending(disp);
    while(x > 0){
      x--;
      XNextEvent(disp, &event);
      switch (event.type){
        case Expose:
          if (event.xexpose.count != 0)
            break;
          System::Engine::instance()->run();
          glXSwapBuffers(disp, win->getWindow());
          break;
        case ConfigureNotify:
          if (event.xconfigure.width != win->getWidth() ||
              event.xconfigure.height != win->getHeight()){
            rend->resizeScene(event.xconfigure.width, event.xconfigure.height);
          }
          break;
        case KeyPress:
        {
          char ascii;
          KeySym key;
          int num = XLookupString(&event.xkey, &ascii, 1, &key, NULL);
          Input::Keyboard::instance()->keyDown(key);
          if (num > 0)
            Input::Keyboard::instance()->ascii(ascii);
          break;
        }
        case KeyRelease:
          Input::Keyboard::instance()->keyUp(XLookupKeysym(&event.xkey, 0));
          break;
        case ButtonPress:
          Input::Mouse::instance()->buttonDown(event.xbutton.x, event.xbutton.y, event.xbutton.button);
          /*
          if (event.xbutton.button == Button1)
            Input::Mouse::instance()->buttonDown(event.xbutton.x, event.xbutton.y, MK_LBUTTON);
          else if (event.xbutton.button == Button2)
            Input::Mouse::instance()->buttonDown(event.xbutton.x, event.xbutton.y, MK_MBUTTON);
          else if (event.xbutton.button == Button3)
            Input::Mouse::instance()->buttonDown(event.xbutton.x, event.xbutton.y, MK_RBUTTON);
          */
          break;
        case ButtonRelease:
          Input::Mouse::instance()->buttonUp(event.xbutton.x, event.xbutton.y, event.xbutton.button);
          break;
        case MotionNotify:
          Input::Mouse::instance()->move(event.xmotion.x, event.xmotion.y, event.xmotion.state);
          break;
        case ClientMessage:
          if (*XGetAtomName(disp, event.xclient.message_type) == *"WM_PROTOCOLS"){
            running = false;
          }
          break;
        default:
          break;
      }
    }
    System::Engine::instance()->run();
    glXSwapBuffers(disp, win->getWindow());
  }
  System::Engine::instance()->shutdown();
  return 0;
}

long GetTickCount(){
  //return clock_gettime(CLOCK_MONOTONIC, NULL);
  //long factor = sysconf(_SC_CLK_TCK);
  return times(NULL);
}
#endif

#include "../window/nativeWindows.h"
#include "script.h"
#include "../gui/console.h"

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

  if (graphics_){
    if (type == "OpenGL")
      rend_ = new ::Graphics::OGLRenderer();
    else if (type == "DirectX"){
#ifdef UNIX
      cerr << "DirectX is not supported on Linux\n";
      exit(-1);
#endif
      rend_ = new ::Graphics::DXRenderer();
    }
    else
      EXIT2("No valid renderer specified in engine.ini");
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
    win_->init("Hero-Engine");
  Input::Keyboard::init();
  Input::Mouse::init(graphics_);
  
  engineMain(argc, argv);
  
  fnt_ = new ::Graphics::Font();
  fnt_->buildFont();
  forms_ = new ::Graphics::Forms();
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
  Input::Keyboard::release();
  Input::Mouse::release();
  SAFE_DELETE(console_);
  fnt_->killFont();
  SAFE_DELETE(fnt_);
  SAFE_DELETE(forms_);
  if (win_)
    win_->kill();
  SAFE_DELETE(win_);
  SAFE_DELETE(rend_);
  Script::kill();
  Log.close();
  delete this;
  exit(0);
}

void System::Engine::run(){
  //handle physics, KI, ...
  console_->update();

  //something with the GUI elements changed, so update here
  for (unsigned i = 0; i < remBut_.size(); i++){
    removeButtonListener(remBut_[i]);
  }
  remBut_.clear();
  if (newIn_.size() != 0 || newBut_.size() != 0 || clear_){
    clear_ = false;
    clearListeners();
    listeners_ = newIn_;
    buttons_ = newBut_;
    newIn_.clear();
    newBut_.clear();
  }

  //render scene
  rend_->renderScene();

  rend_->resetModelView();
  rend_->ortho(1024, 768);
  rend_->translate(-512, -384, 0);
  rend_->blendFunc(Graphics::BLEND_SRC_ALPHA, Graphics::BLEND_ONE);
  rend_->enableBlend(true);
  
  //render GUI-elements
  list< ::Gui::InputField*>::iterator iter;
  for (iter = listeners_.begin(); iter != listeners_.end(); iter++){
    (*iter)->render();
  }
  
  list< ::Gui::Button*>::iterator iter2;
  for (iter2 = buttons_.begin(); iter2 != buttons_.end(); iter2++){
    (*iter2)->render();
  }

  System::Engine::instance()->getConsole()->render();

  fnt_->render();

  rend_->enableBlend(false);

#ifdef WIN32
  if (rend_->getRenderType() == Graphics::DirectX){
    Graphics::DXRenderer* dxr = dynamic_cast<Graphics::DXRenderer*>(rend_);
    // End the scene
    dxr->getDevice()->EndScene();

    // Present the backbuffer contents to the display
    dxr->getDevice()->Present( NULL, NULL, NULL, NULL );
  }
#endif

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
  list< ::Gui::InputField* >::iterator iter;
  for (iter = listeners_.begin(); iter != listeners_.end(); iter++){
    delete (*iter);
  }
  list< ::Gui::Button* >::iterator iter2;
  for (iter2 = buttons_.begin(); iter2 != buttons_.end(); iter2++){
    delete (*iter2);
  }
  listeners_.clear();
  buttons_.clear();
  fnt_->clear();
}


// remove the input listener at position idx
void System::Engine::removeInputListener(int idx){
  list< ::Gui::InputField*>::iterator iter = listeners_.begin();
  for (int i = 0; i < idx; i++){
    iter++;
  }
  listeners_.erase(iter);
}

// remove the button listener at position idx
void System::Engine::removeButtonListener(int idx, bool immediate){
  if (!immediate){
    remBut_.push_back(idx);
    return;
  }
  list< ::Gui::Button*>::iterator iter = buttons_.begin();
  for (int i = 0; i < idx; i++){
    iter++;
  }
  delete *iter;
  buttons_.erase(iter);
}


// remove the button listener with a certain name
void System::Engine::removeButtonListener(const string& name){
  list< ::Gui::Button*>::iterator iter = buttons_.begin();
  for (unsigned i = 0; i < buttons_.size(); i++){
    if((*iter)->getText() == name){
      delete *iter;
      buttons_.erase(iter);
      break;
    }
    iter++;
  }
}


// remove all button listeners beginning at idx
void System::Engine::removeButtonListeners(int idx){
  list< ::Gui::Button*>::iterator iter = buttons_.begin();
  for (int i = 0; i < idx; i++){
    iter++;
  }
  while (iter != buttons_.end()){
    delete *iter;
    iter = buttons_.erase(iter);
  }
}
