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
}

void Keyboard::init(){
  key_ = new Keyboard();
}

void Keyboard::keyDown(int key){
  if (key == KEY_ESCAPE){
    Gui::Console* cons = System::Engine::instance()->getConsole();
    //quit console
    if (cons->isActive()){
      cons->toggle();
      System::Engine::instance()->getActiveInput()->clear();
      System::Engine::instance()->setActiveInput(NULL);
      return;
    }
  }
  InputField* input = System::Engine::instance()->getActiveInput();
  //input goes to input field
  if (input != NULL){
    //input finished
    if (key == KEY_RETURN){
      input->removeChar();
      input->end();
      //normal input ==> single line
      if (!System::Engine::instance()->getConsole()->isActive())
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
      System::Engine::instance()->getConsole()->Up();
    }
    else if (key == KEY_DOWN){
      System::Engine::instance()->getConsole()->Down();
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
  if (keyUpCB_){
    keyUpCB_(key);
  }
}

void Keyboard::ascii(unsigned char key){
  //accept only printable asciis
  if (key <= 31 /*|| key >= 127*/)
    return;
  InputField* input = System::Engine::instance()->getActiveInput();
  //input goes to input field
  if (input != NULL){
    input->removeChar();
    input->addChar(key);
    input->addChar('_');
    return;
  }
  //open console
  if (key == '~'){
    System::Engine::instance()->getConsole()->toggle();
    System::Engine::instance()->setActiveInput(System::Engine::instance()->getConsole()->GetInputField());
  }

  if (asciiCB_){
    asciiCB_(key);
  }
  //::System::Log << (unsigned char)key << " ";
}

