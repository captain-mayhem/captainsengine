#include <cstdlib>
#include "../system/engine.h"
#include "../gui/gui.h"
#include "../input/keyboard.h"

using Gui::InputField;

using namespace Input;

Keyboard* Keyboard::key_ = NULL;

Keyboard::Keyboard(){
  keyDownCB_ = NULL;
  keyUpCB_ = NULL;
  asciiCB_ = NULL;
}

void Keyboard::init(){
  key_ = new Keyboard();
}

void Keyboard::keyDown(int key){
  InputField* input = System::Engine::instance()->getActiveInput();
  //input goes to input field
  if (input != NULL){
    //input finished
    if (key == KEY_RETURN){
      input->removeChar();
      input->end();
      //normal input ==> single line
      //if (!consol.isActive())
        System::Engine::instance()->setActiveInput(NULL);
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
    }
    else if (key == KEY_DOWN){
    }
  }
  if (keyDownCB_){
    keyDownCB_(key);
  }
  if (key == KEY_ESCAPE)
    EXIT();
}

void Keyboard::keyUp(int key){
  if (keyUpCB_){
    keyUpCB_(key);
  }
}

void Keyboard::ascii(unsigned char key){
  //accept only printable asciis
  if (key <= 31 || key >= 127)
    return;
  InputField* input = System::Engine::instance()->getActiveInput();
  //input goes to input field
  if (input != NULL){
    input->removeChar();
    input->addChar(key);
    input->addChar('_');
  }
  if (asciiCB_){
    asciiCB_(key);
  }
  //::System::Log << (unsigned char)key << " ";
}

