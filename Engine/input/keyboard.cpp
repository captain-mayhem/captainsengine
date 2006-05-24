#include <cstdlib>
#include "../system/engine.h"
#include "../input/keyboard.h"

namespace Input{

Keyboard* Keyboard::key_ = NULL;

Keyboard::Keyboard(){

}

void Keyboard::init(){
  key_ = new Keyboard();
}

void Keyboard::keyDown(int key){
  //cerr << key;
  if (key == KEY_ESCAPE)
    EXIT();
}

void Keyboard::keyUp(int key){

}

}
