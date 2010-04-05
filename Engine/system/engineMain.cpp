
#include "engine.h"
#include "../input/keyboard.h"
#include "../input/mouse.h"

extern void engineMain(int argc, char** argv);
extern void (*internalEngineMain)(int argc, char** argv);

#ifdef WIN32

#include "../window/nativeWindows.h"
#include <windows.h>



int WINAPI WinMain(HINSTANCE instance, HINSTANCE oldinstance, LPTSTR cmdline, int cmdShow){
  internalEngineMain = engineMain;
  MSG msg;
  char* argv[2];
  argv[0] = "Captains Game Engine";
#ifdef UNDER_CE
  char tmp[1024];
  wcstombs(tmp, cmdline, 1024);
  argv[1] = tmp;
#else
  argv[1] = cmdline;
#endif
  CGE::Engine::init();
  CGE::Engine::instance()->startup(2, argv);
  //Enter gameloop
  while(CGE::Engine::instance() != NULL){
    while (PeekMessage(&msg,NULL,0,0,PM_REMOVE)){
      if(msg.message == WM_QUIT)
        break;
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    if (CGE::Engine::instance()){
      //Do not use WM_MouseMove-Event, because it causes the cursor to freeze
      POINT p;
      GetCursorPos(&p);
      ScreenToClient(static_cast<Windows::WindowsWindow*>(CGE::Engine::instance()->getWindow())->getHandle(), &p);
      Input::Mouse::instance()->move(p.x, p.y, 0);
      CGE::Engine::instance()->run();
    }
  }
  //CGE::Engine::instance()->shutdown();
  return (int)msg.wParam;
}
#endif

#include "../window/nativeLinux.h"
#include "../input/keyboard.h"
#include "../renderer/renderer.h"
#include "../renderer/font.h"
#include "../gui/gui.h"

#ifdef UNIX
#include <X11/Xlib.h>
#include <GL/glx.h>

int main(int argc, char** argv){
  internalEngineMain = engineMain;
  CGE::Engine::init();
  CGE::Engine::instance()->startup(argc, argv);
  XEvent event;
  bool running = true;
  Windows::X11Window* win = dynamic_cast<Windows::X11Window*>(CGE::Engine::instance()->getWindow());
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
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
          CGE::Engine::instance()->run();
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
    CGE::Engine::instance()->run();
  }
  CGE::Engine::instance()->shutdown();
  return 0;
}

long GetTickCount(){
  //return clock_gettime(CLOCK_MONOTONIC, NULL);
  //long factor = sysconf(_SC_CLK_TCK);
  return times(NULL);
}
#endif

