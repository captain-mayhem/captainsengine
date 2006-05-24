#include "../renderer/OGLrenderer.h"
#include "../renderer/DXrenderer.h"
#include "engine.h"

#ifdef WIN32

#include <windows.h>

int WINAPI WinMain(HINSTANCE instance, HINSTANCE oldinstance, LPSTR cmdline, int cmdShow){
  MSG msg;
  Engine::Engine::init();
  Engine::Engine::instance()->startup();
  //Enter gameloop
  while(true){
    while (PeekMessage(&msg,NULL,0,0,PM_REMOVE)){
      if(msg.message == WM_QUIT)
        break;
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    Engine::Engine::instance()->run();
    if (Engine::Engine::instance()->getRenderer()->getRenderType() == Renderer::OpenGL){
      SwapBuffers(dynamic_cast<Renderer::OGLRenderer*>(Engine::Engine::instance()->getRenderer())->getDevice());
    }
  }
  Engine::Engine::instance()->shutdown();
  return (int)msg.wParam;
}
#endif

#include "../window/nativeLinux.h"
#include "../input/keyboard.h"

#ifdef UNIX
#include <X11/Xlib.h>
#include <GL/glx.h>

int main(int argc, char** argv){
  Engine::Engine::init();
  Engine::Engine::instance()->startup();
  XEvent event;
  bool running = true;
  Windows::X11Window* win = dynamic_cast<Windows::X11Window*>(Engine::Engine::instance()->getWindow());
  Renderer::Renderer* rend = Engine::Engine::instance()->getRenderer();
  Display* disp = win->getDisplay();
  
  while(running){
    int x = XPending(disp);
    while(x > 0){
      XNextEvent(disp, &event);
      switch (event.type){
        case Expose:
          if (event.xexpose.count != 0)
            break;
          Engine::Engine::instance()->run();
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
    Engine::Engine::instance()->run();
    glXSwapBuffers(disp, win->getWindow());
  }
  Engine::Engine::instance()->shutdown();
  return 0;
}
#endif

#include "../window/nativeWindows.h"
#include "script.h"

ofstream Engine::Log("engine.log");

Engine::Engine* Engine::Engine::eng = NULL;

Engine::Engine::Engine(){
  win_ = NULL;
  rend_ = NULL;
  Log << "Engine instance created\n";
}

void Engine::Engine::init(){
  eng = new Engine();
}

void Engine::Engine::startup(){
  Script::init();
  Script::instance()->initEnv();
  string type = Script::instance()->getStringSetting("renderer");
  if (type == "OpenGL")
    rend_ = new Renderer::OGLRenderer();
  else if (type == "DirectX")
    rend_ = new ::Renderer::DXRenderer();
  else
    EXIT2("No valid renderer specified in engine.ini");
#ifdef WIN32
  win_ = new ::Windows::WindowsWindow(rend_);
#endif
#ifdef UNIX
  win_ = new ::Windows::X11Window(rend_);
#endif
  win_->init("ACE-Engine");
  Input::Keyboard::init();
}

void Engine::Engine::shutdown(){
  Log << "engine shutting down\n";
  Input::Keyboard::release();
  if (win_)
    win_->kill();
  SAFE_DELETE(win_);
  SAFE_DELETE(rend_);
  Script::kill();
  Log.close();
  delete this;
  exit(0);
}

void Engine::Engine::run(){
  rend_->renderScene();
}
  

