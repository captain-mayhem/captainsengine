#include <cstdlib>
#include "../system/engine.h"
#include "../gui/gui.h"
#include "../gui/console.h"
#include "../input/keyboard.h"

using Gui::InputField;

using namespace Input;

Keyboard* Keyboard::key_ = nullptr;

Keyboard::Keyboard() : mBuiltInConsole(true){
}

void Keyboard::init(){
  key_ = new Keyboard();
}

void Keyboard::keyDown(int key){
  pressedKeys_.remove(key);
  pressedKeys_.push_back(key);
  if (key == KEY_ESCAPE){
    Gui::Console* cons = CGE::Engine::instance()->getConsole();
    //quit console
    if (cons->isActive()){
      cons->toggle();
      if (CGE::Engine::instance()->getActiveInput()){
        CGE::Engine::instance()->getActiveInput()->clear();
        CGE::Engine::instance()->setActiveInput(nullptr);
      }
      return;
    }
  }
  InputField* input = CGE::Engine::instance()->getActiveInput();
  //input goes to input field
  if (input != nullptr){
    //input finished
    if (key == KEY_RETURN){
      input->removeChar();
      input->end();
      //normal input ==> single line
      if (!CGE::Engine::instance()->getConsole()->isActive())
        CGE::Engine::instance()->setActiveInput(nullptr);
      return;
    }
    //delete one char
    else if (key == KEY_BACKSPACE){
      input->removeChar();
      input->removeChar();
      input->addChar('_');
      return;
    }
    //history
    else if (key == KEY_UP){
      CGE::Engine::instance()->getConsole()->Up();
    }
    else if (key == KEY_DOWN){
      CGE::Engine::instance()->getConsole()->Down();
    }
    return;
  }
  
  CGE::IApplication* app = CGE::Engine::instance()->getApplication();
  if (app)
    app->onKeyDown(key);
}

void Keyboard::keyUp(int key){
  pressedKeys_.remove(key);
  CGE::IApplication* app = CGE::Engine::instance()->getApplication();
  if (app)
    app->onKeyUp(key);
}

void Keyboard::ascii(unsigned char key){
  //accept only printable asciis
  if (key <= 31 /*|| key >= 127*/)
    return;
  InputField* input = CGE::Engine::instance()->getActiveInput();
  //input goes to input field
  if (input != nullptr){
    input->removeChar();
    input->addChar(key);
    input->addChar('_');
    return;
  }
  //open console
  if (mBuiltInConsole && key == '~'){
    CGE::Engine::instance()->getConsole()->toggle();
    CGE::Engine::instance()->setActiveInput(CGE::Engine::instance()->getConsole()->GetInputField());
  }

  CGE::IApplication* app = CGE::Engine::instance()->getApplication();
  if (app)
    app->onChar(key);
}

bool Keyboard::isPressed(int key) const{
  for (int pressed : pressedKeys_){
    if (pressed == key)
      return true;
  }
  return false;
}

