#if defined(LINUX)
#include <X11/Xutil.h>
#endif
#include "../system/engine.h"
#include "../window/window.h"
#include "../window/nativeLinux.h"
#include "../window/nativeWindows.h"
#include "../gui/gui.h"
#include "../gui/console.h"
#include "../math/vector.h"
#include "../input/mouse.h"
#include "../platform/platform_internal.h"

using CGE::AppWindow;
using Gui::InputField;
using Gui::Button;;
using Gui::Console;
using Gui::GuiElement;
using CGE::Vector2D;

using namespace Input;

Mouse* Mouse::mouse_ = NULL;

Mouse::Mouse(){
  mousePointer_ = true;
  pressed_[0] = false;
  pressed_[1] = false;
  pressed_[2] = false;
#ifdef LINUX
  graphics_ = false;
#endif
}

Mouse::~Mouse(){
#ifdef WIN32
  //ReleaseCapture();
#endif
#if defined(LINUX)
  if (!graphics_)
    return;
  CGE::X11Window* wnd = dynamic_cast<CGE::X11Window*>(CGE::Engine::instance()->getWindow());
  XFreeCursor(wnd->getDisplay(), invCursor_);
#endif
}

void Mouse::init(bool hasGraphics){
  mouse_ = new Mouse();
#ifdef WIN32
  //Windows::WindowsWindow* wnd = dynamic_cast<Windows::WindowsWindow*>(CGE::Engine::instance()->getWindow());
  //SetCapture(wnd->getHandle());
#endif
#if defined(LINUX)
  if (!hasGraphics)
    return;
  mouse_->graphics_ = true;
  CGE::X11Window* wnd = dynamic_cast<CGE::X11Window*>(CGE::Engine::instance()->getWindow());
  //create invisible cursor
  char data[8] = {0,0,0,0,0,0,0,0};
  XImage* img = XCreateImage(wnd->getDisplay(), DefaultVisual(wnd->getDisplay(), wnd->getScreen()),
      1, XYBitmap, 0, data, 8, 8, 8, 1);
  img->byte_order = MSBFirst;
  img->bitmap_bit_order = MSBFirst;
  Pixmap pm = XCreatePixmap(wnd->getDisplay(), RootWindow(wnd->getDisplay(), wnd->getScreen()), 8, 8, 1);
  XGCValues gcval;
  gcval.function = GXcopy;
  gcval.foreground = ~0;
  gcval.background = 0;
  gcval.plane_mask = AllPlanes;
  GC gccursor = XCreateGC(wnd->getDisplay(), pm, GCFunction|GCForeground|GCBackground|GCPlaneMask,&gcval);
  XPutImage(wnd->getDisplay(), pm, gccursor, img, 0, 0, 0, 0, 8, 8);
  XFreeGC(wnd->getDisplay(), gccursor);
  //XDestroyImage(img);

  XColor black = {0, 0, 0, 0};
  XColor white = {0xffff, 0xffff, 0xffff, 0xffff};
  mouse_->invCursor_ = XCreatePixmapCursor(wnd->getDisplay(), pm, pm, &black, &white, 0, 0);
  XFreePixmap(wnd->getDisplay(), pm);
#endif
}

void Mouse::buttonDown(int x, int y, int button){
  //CGE::Log << "Click: " << x << " " << y << " " << button<< "\n";
  
  //map to virtual resolution
  CGE::VirtualScreen& screen = CGE::Engine::instance()->getVirtualScreen();
  clickPos_ = screen.toVirtual(x, y);
  
  gui_click_ = false;
/*
  //click on a GUI-element?
  list<InputField*>::iterator iter;
  for (iter = CGE::Engine::instance()->getInputFields().begin(); iter != CGE::Engine::instance()->getInputFields().end(); iter++){
    if ((*iter)->isClicked(Vector2D(clickPos_.x, screen.getHeight()-clickPos_.y))){
      //set only input focus if console is not active
      if (!CGE::Engine::instance()->getConsole()->isActive()){
        //another input field was active, so remove cursor
        CGE::Engine::instance()->setActiveInput(*iter);
        gui_click_ = true;
        break;
      }
    }
  }
*/
  list<GuiElement*>::iterator iter2;
  for (iter2 = CGE::Engine::instance()->getGuiElements().begin(); iter2 != CGE::Engine::instance()->getGuiElements().end(); iter2++){
    if ((*iter2)->isClicked(Vector2D(clickPos_.x, screen.getHeight()-clickPos_.y))){
      /*//is it an input field?
      InputField* inp = dynamic_cast<InputField*>(*iter2);
      if ((*iter2)->getType() == Gui::InputFieldT && inp){
        //set only input focus if console is not active
        if (!CGE::Engine::instance()->getConsole()->isActive()){
          (*iter2)->process();
          //another input field was active, so remove cursor
          //CGE::Engine::instance()->setActiveInput(inp);
          gui_click_ = true;
          break;
        }
      }*/
      //allow only clicks if the console is deactivated
      if (!CGE::Engine::instance()->getConsole()->isActive()){
        /*InputField* input = CGE::Engine::instance()->getActiveInput();
        if (input != NULL){
          input->removeChar();
        }
        CGE::Engine::instance()->setActiveInput(NULL);*/
        (*iter2)->process();
        gui_click_ = true;
        break;
      }
    }
  }

  if (!gui_click_){
    switch(button){
      case MB_LEFT:
        pressed_[0] = true;
        break;
      case MB_MIDDLE:
        pressed_[1] = true;
        break;
      case MB_RIGHT:
        pressed_[2] = true;
        break;
    };
  }
    
  if (CGE::IApplication* app = CGE::Engine::instance()->getApplication())
    app->onMouseDown(clickPos_.x, clickPos_.y, button);
}

void Mouse::buttonUp(int x, int y, int button){
  Vector2D pos = CGE::Engine::instance()->getVirtualScreen().toVirtual(x, y);
  switch(button){
    case MB_LEFT:
      pressed_[0] = false;
      break;
    case MB_MIDDLE:
      pressed_[1] = false;
      break;
    case MB_RIGHT:
      pressed_[2] = false;
      break;
  };

  if (CGE::IApplication* app = CGE::Engine::instance()->getApplication())
    app->onMouseUp(pos.x, pos.y, button);
}

void Mouse::doubleClick(int x, int y, int button){
  clickPos_ = CGE::Engine::instance()->getVirtualScreen().toVirtual(x, y);
  if (CGE::IApplication* app = CGE::Engine::instance()->getApplication())
    app->onDoubleClick(clickPos_.x, clickPos_.y, button);
}

void Mouse::move(int x, int y, int buttons){
  //map to virtual resolution
  if (!CGE::Engine::instance())
    return;
  AppWindow* wnd = CGE::Engine::instance()->getWindow();
  if (!wnd)
    return;
  mousePos_ = CGE::Engine::instance()->getVirtualScreen().toVirtual(x, y);
  if (CGE::IApplication* app = CGE::Engine::instance()->getApplication())
    app->onMouseMove(mousePos_.x, mousePos_.y, buttons);
}

void Mouse::wheel(int x, int y, int delta){
  if (!CGE::Engine::instance())
    return;
  AppWindow* wnd = CGE::Engine::instance()->getWindow();
  if (!wnd)
    return;
  mousePos_ = CGE::Engine::instance()->getVirtualScreen().toVirtual(x, y);
  if (CGE::IApplication* app = CGE::Engine::instance()->getApplication())
    app->onMouseWheel(mousePos_.x, mousePos_.y, delta);
}

void Mouse::showCursor(bool visible){
  if (mousePointer_ == visible)
    return;
  mousePointer_ = visible;
#ifdef WIN32
  if (mousePointer_)
    ShowCursor(1);
  else
    ShowCursor(0);
#endif
#if defined(LINUX)
  CGE::X11Window* wnd = dynamic_cast<CGE::X11Window*>(CGE::Engine::instance()->getWindow());
  if (mousePointer_){
    CGE::PlatformInternal::applyCursor();
  }
  else{
    XDefineCursor(wnd->getDisplay(), wnd->getWindow(), invCursor_);
  }
#endif
}

void Mouse::setMousePos(int x, int y){
  CGE::Vec2i win = CGE::Engine::instance()->getVirtualScreen().toWindow(x, y);
  x = win.x;
  y = win.y;
#ifdef WIN32
  POINT point;
  point.x = x;
  point.y = y;
  ClientToScreen((HWND)static_cast<CGE::WindowsWindow*>(CGE::Engine::instance()->getWindow())->getHandle(), &point);
  SetCursorPos(point.x, point.y);
#endif
#if defined(LINUX)
  CGE::X11Window* wnd = dynamic_cast<CGE::X11Window*>(CGE::Engine::instance()->getWindow());
  XWarpPointer(wnd->getDisplay(), None, wnd->getWindow(), 0, 0, 0, 0, x, y);
  XEvent useless;
  XMaskEvent(wnd->getDisplay(), PointerMotionMask, &useless);
#endif
}

// is a button pressed
bool Mouse::isPressed(int button){
  switch(button){
    case MB_LEFT:
      return pressed_[0];
    case MB_MIDDLE:
      return pressed_[1];
    case MB_RIGHT:
      return pressed_[2];
  };
  return false;
}
