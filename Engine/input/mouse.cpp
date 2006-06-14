#include "../system/engine.h"
#include "../input/mouse.h"

using namespace Input;

Mouse* Mouse::mouse_ = NULL;

Mouse::Mouse(){

}

void Mouse::init(){
    mouse_ = new Mouse();
}

void Mouse::buttonDown(int x, int y, int button){
}

void Mouse::buttonUp(int x, int y, int button){

}

void Mouse::move(int x, int y, int buttons){
}

