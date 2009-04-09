#include <cstdlib>
#include "../system/engine.h"
#include "../gui/gui.h"
#include "../gui/console.h"
#include "../input/keyboard.h"

using Gui::InputField;

using namespace Input;

Keyboard* Keyboard::key_ = NULL;

Keyboard::Keyboard(){
  keyDownCB_ = NULL;
  keyUpCB_ = NULL;
  asciiCB_ = NULL;
  keyPressedCB_ = NULL;
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
      CGE::Engine::instance()->getActiveInput()->clear();
      CGE::Engine::instance()->setActiveInput(NULL);
      return;
    }
  }
  InputField* input = CGE::Engine::instance()->getActiveInput();
  //input goes to input field
  if (input != NULL){
    //input finished
    if (key == KEY_RETURN){
      input->removeChar();
      input->end();
      //normal input ==> single line
      if (!CGE::Engine::instance()->getConsole()->isActive())
        CGE::Engine::instance()->setActiveInput(NULL);
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
  
  if (keyDownCB_){
    keyDownCB_(key);
  }
  else{
    if (key == KEY_ESCAPE)
      EXIT();
  }
}

void Keyboard::keyUp(int key){
  pressedKeys_.remove(key);
  if (keyUpCB_){
    keyUpCB_(key);
  }
}

void Keyboard::ascii(unsigned char key){
  //accept only printable asciis
  if (key <= 31 /*|| key >= 127*/)
    return;
  InputField* input = CGE::Engine::instance()->getActiveInput();
  //input goes to input field
  if (input != NULL){
    input->removeChar();
    input->addChar(key);
    input->addChar('_');
    return;
  }
  //open console
  if (key == '~'){
    CGE::Engine::instance()->getConsole()->toggle();
    CGE::Engine::instance()->setActiveInput(CGE::Engine::instance()->getConsole()->GetInputField());
  }

  if (asciiCB_){
    asciiCB_(key);
  }
  //::CGE::Log << (unsigned char)key << " ";
}

void Keyboard::processPressedKeys(float diffTime){
  if (keyPressedCB_){
    for (std::list<int>::iterator iter = pressedKeys_.begin(); iter != pressedKeys_.end(); ++iter){
      keyPressedCB_(*iter, diffTime);
    }
  }
}

