
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
  std::vector<std::string> args;
  std::vector<const char*> argv;
  args.push_back("Captains Game Engine");
#ifdef UNDER_CE
  char tmp[1024];
  wcstombs(tmp, cmdline, 1024);
  args.push_back(tmp);
#else
  int len = strlen(cmdline);
  std::string curr;
  for (int i = 0; i < len; ++i){
    if (cmdline[i] == ' ' && !curr.empty()){
      args.push_back(curr);
      curr.clear();
    }
    else
      curr.append(1, cmdline[i]);
  }
  if (!curr.empty()){
    args.push_back(curr);
    curr.clear();
  }
#endif
  argv.resize(args.size());
  for (unsigned i = 0; i < args.size(); ++i){
    argv[i] = args[i].c_str();
  }
  CGE::Engine::init();
  CGE::Engine::instance()->startup(argv.size(), (char**)&argv[0]);
  //Enter gameloop
  while(CGE::Engine::instance() != NULL && !CGE::Engine::instance()->isShutdownRequested()){
    while (PeekMessage(&msg,NULL,0,0,PM_REMOVE)){
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      if(msg.message == WM_QUIT)
        break;
    }
    if (CGE::Engine::instance() && !CGE::Engine::instance()->isShutdownRequested()){
      //Do not use WM_MouseMove-Event, because it causes the cursor to freeze
      POINT p;
      GetCursorPos(&p);
      ScreenToClient((HWND)static_cast<Windows::WindowsWindow*>(CGE::Engine::instance()->getWindow())->getHandle(), &p);
      Input::Mouse::instance()->move(p.x, p.y, 0);
      CGE::Engine::instance()->run();
    }
  }
  if (CGE::Engine::instance()->isShutdownRequested())
    CGE::Engine::instance()->shutdown();
  return 0;
}
#endif

#include "../window/nativeLinux.h"
#include "../input/keyboard.h"
#include "../renderer/renderer.h"
#include "../renderer/font.h"
#include "../gui/gui.h"

#if defined(LINUX) && !defined(NO_X11)
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

  double lastClick = 0;
  while(running && CGE::Engine::instance() != NULL && !CGE::Engine::instance()->isShutdownRequested()){
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
          if (event.xbutton.button > Button3){
            Input::Mouse::instance()->wheel(event.xbutton.x, event.xbutton.y, event.xbutton.button == Button4 ? 1 : -1);
          }
          else{
            double newClick = getTime();
            if (newClick - lastClick < 0.5){
              Input::Mouse::instance()->doubleClick(event.xbutton.x, event.xbutton.y, event.xbutton.button);
              lastClick = 0;
            }
            else{
              lastClick = getTime();
              Input::Mouse::instance()->buttonDown(event.xbutton.x, event.xbutton.y, event.xbutton.button);
            }
          }
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
          if (event.xbutton.button <= Button3 && lastClick != 0)
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
#endif

