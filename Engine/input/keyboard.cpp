#include <cstdlib>
#include "../system/engine.h"
#include "../ui/console.h"
#include "../ui/imgui_impl_cge_platform.h"
#include "../input/keyboard.h"

using namespace Input;

Keyboard* Keyboard::key_ = nullptr;

namespace{

//! ImGui takes the keyboard (e.g. for a text field): the application doesn't get it
bool imguiWantsKeyboard(){
  return ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureKeyboard;
}

}

Keyboard::Keyboard() : mBuiltInConsole(true){
}

void Keyboard::init(){
  key_ = new Keyboard();
}

void Keyboard::keyDown(int key){
  pressedKeys_.remove(key);
  pressedKeys_.push_back(key);
  //quit console
  CGE::Console* cons = CGE::Engine::instance()->getConsole();
  if (key == KEY_ESCAPE && cons && cons->isActive()){
    cons->toggle();
    return;
  }
  if (imguiWantsKeyboard())
    return;
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
  //open console (the character doesn't go into its input line)
  CGE::Console* cons = CGE::Engine::instance()->getConsole();
  if (mBuiltInConsole && key == '~' && cons && !cons->isActive() && !imguiWantsKeyboard()){
    cons->toggle();
    return;
  }
  ImGui_ImplCGE_Platform_AddChar(key);
  if (imguiWantsKeyboard())
    return;
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

