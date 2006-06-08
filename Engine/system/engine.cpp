#include "../renderer/OGLrenderer.h"
#include "../renderer/DXrenderer.h"
#include "../renderer/dummyrenderer.h"
#include "engine.h"

extern void engineMain(int argc, char** argv);

#ifdef WIN32

#include <windows.h>

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

#ifdef UNIX
#include <X11/Xlib.h>
#include <GL/glx.h>

int main(int argc, char** argv){
  System::Engine::init();
  System::Engine::instance()->startup();
  XEvent event;
  bool running = true;
  Windows::X11Window* win = dynamic_cast<Windows::X11Window*>(System::Engine::instance()->getWindow());
  Graphics::Renderer* rend = System::Engine::instance()->getRenderer();
  Display* disp = win->getDisplay();
  
  while(running){
    int x = XPending(disp);
    while(x > 0){
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
          Input::Keyboard::instance()->keyDown(XLookupKeysym(&event.xkey, 0));
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
#endif

#include "../window/nativeWindows.h"
#include "script.h"

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
  Input::Keyboard::init();
  isUp_ = true;
  engineMain(argc, argv);
  if (graphics_)
    win_->init("Hero-Engine");
  fnt_ = new ::Graphics::Font();
  fnt_->buildFont();
  //isUp_ = true;
}

void System::Engine::shutdown(){
  if (!isUp_)
    return;
  Log << "engine shutting down\n";
  isUp_ = false;
  Input::Keyboard::release();
  fnt_->killFont();
  SAFE_DELETE(fnt_);
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
  
  //render scene
  rend_->renderScene();

  //calculate frame rate
  double currentTime;
  if (maxFramerate_ == 0){
    currentTime = GetTickCount()*0.001;
    frameInterval_ = currentTime-frameTime_;
    frameTime_ = currentTime;
  }
  else{
    while(((currentTime = GetTickCount()*0.001)-frameTime_) <= 1.0/maxFramerate_);
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
  

