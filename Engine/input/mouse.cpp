#include "../system/engine.h"
#include "../window/window.h"
#include "../gui/gui.h"
#include "../input/mouse.h"

using Windows::AppWindow;
using Gui::InputField;

using namespace Input;

Mouse* Mouse::mouse_ = NULL;

Mouse::Mouse(){
  buttonDownCB_ = NULL;
  buttonUpCB_ = NULL;
  moveCB_ = NULL;
}

void Mouse::init(){
    mouse_ = new Mouse();
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
      //if (!consol.isActive()){
        //another input field was active, so remove cursor
        System::Engine::instance()->setActiveInput(*iter);
        gui_click_ = true;
        break;
      //}
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

