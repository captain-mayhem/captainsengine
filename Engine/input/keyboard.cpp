#include <cstdlib>
#include "../system/engine.h"
#include "../input/keyboard.h"

using namespace Input;

Keyboard* Keyboard::key_ = NULL;

Keyboard::Keyboard(){

}

void Keyboard::init(){
  key_ = new Keyboard();
}

void Keyboard::keyDown(int key){
  //cerr << key;
  ::System::Log << (unsigned char)key << " ";
  //::System::Log << (unsigned char)__toascii(key) << "\n";
  if (key == KEY_ESCAPE)
    EXIT();
}

void Keyboard::keyUp(int key){
  
}

void Keyboard::ascii(unsigned char key){
  ::System::Log << (unsigned char)key << " ";
}

