#ifdef UNIX
#include <X11/Xutil.h>
#endif
#include "../system/engine.h"
#include "../window/window.h"
#include "../window/nativeLinux.h"
#include "../gui/gui.h"
#include "../gui/console.h"
#include "../math/vector.h"
#include "../input/mouse.h"

using Windows::AppWindow;
using Gui::InputField;
using Gui::Button;;
using Gui::Console;
using Math::Vector2D;

using namespace Input;

Mouse* Mouse::mouse_ = NULL;

Mouse::Mouse(){
  buttonDownCB_ = NULL;
  buttonUpCB_ = NULL;
  moveCB_ = NULL;
  mousePointer_ = true;
  graphics_ = false;
}

Mouse::~Mouse(){
#ifdef UNIX
  if (!graphics_)
    return;
  Windows::X11Window* wnd = dynamic_cast<Windows::X11Window*>(System::Engine::instance()->getWindow());
  XFreeCursor(wnd->getDisplay(), invCursor_);
#endif
}

void Mouse::init(bool hasGraphics){
  mouse_ = new Mouse();
#ifdef UNIX
  if (!hasGraphics)
    return;
  mouse_->graphics_ = true;
  Windows::X11Window* wnd = dynamic_cast<Windows::X11Window*>(System::Engine::instance()->getWindow());
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
  //System::Log << "Click: " << x << " " << y << " " << button<< "\n";
  
  //map to virtual resolution
  AppWindow* wnd = System::Engine::instance()->getWindow();
  clickPos_.x = (int)((float)x/wnd->getWidth()*SCREENWIDTH);
  clickPos_.y = (int)((float)y/wnd->getHeight()*SCREENHEIGHT);
  
  gui_click_ = false;

  //click on a GUI-element?
  list<InputField*>::iterator iter;
  for (iter = System::Engine::instance()->getInputFields().begin(); iter != System::Engine::instance()->getInputFields().end(); iter++){
    if ((*iter)->isHit(Vector2D(clickPos_.x, SCREENHEIGHT-clickPos_.y))){
      //set only input focus if console is not active
      if (!System::Engine::instance()->getConsole()->isActive()){
        //another input field was active, so remove cursor
        System::Engine::instance()->setActiveInput(*iter);
        gui_click_ = true;
        break;
      }
    }
  }

  list<Button*>::iterator iter2;
  for (iter2 = System::Engine::instance()->getButtons().begin(); iter2 != System::Engine::instance()->getButtons().end(); iter2++){
    if ((*iter2)->isClicked(Vector2D(clickPos_.x, SCREENHEIGHT-clickPos_.y))){
      //allow only clicks if the console is deactivated
      if (!System::Engine::instance()->getConsole()->isActive()){
        InputField* input = System::Engine::instance()->getActiveInput();
        if (input != NULL){
          input->removeChar();
        }
        System::Engine::instance()->setActiveInput(NULL);
        (*iter2)->process();
        gui_click_ = true;
        break;
      }
    }
  }
    
  if (buttonDownCB_)
    buttonDownCB_(x, y, button);
}

void Mouse::buttonUp(int x, int y, int button){
  if (buttonUpCB_)
    buttonUpCB_(x, y, button);
}

void Mouse::move(int x, int y, int buttons){
  //System::Log << "Move: " << x << " " << y << "\n";
  if (moveCB_)
    moveCB_(x, y, buttons);
}

void Mouse::showCursor(bool visible){
  mousePointer_ = visible;
#ifdef WIN32
  if (mousePointer_)
    ShowCursor(1);
  else
    ShowCursor(0);
#endif
#ifdef UNIX
  Windows::X11Window* wnd = dynamic_cast<Windows::X11Window*>(System::Engine::instance()->getWindow());
  if (mousePointer_){
    XDefineCursor(wnd->getDisplay(), wnd->getWindow(), None);
  }
  else{
    XDefineCursor(wnd->getDisplay(), wnd->getWindow(), invCursor_);
  }
#endif
}
