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

void Keyboard::character(unsigned int ch){
  //only printable characters: no C0 or C1 controls, no DEL, no surrogates
  if (ch < 0x20 || (ch >= 0x7f && ch < 0xa0) || (ch >= 0xd800 && ch < 0xe000) || ch > 0x10ffff)
    return;
  //open console (the character doesn't go into its input line)
  CGE::Console* cons = CGE::Engine::instance()->getConsole();
  if (mBuiltInConsole && ch == '~' && cons && !cons->isActive() && !imguiWantsKeyboard()){
    cons->toggle();
    return;
  }
  ImGui_ImplCGE_Platform_AddChar(ch);
  if (imguiWantsKeyboard())
    return;
  CGE::IApplication* app = CGE::Engine::instance()->getApplication();
  if (app)
    app->onChar(ch);
}

void Keyboard::text(const char* utf8, size_t length){
  const unsigned char* s = (const unsigned char*)utf8;
  size_t i = 0;
  while (i < length){
    unsigned int ch;
    size_t len;
    if (s[i] < 0x80){
      ch = s[i];
      len = 1;
    }
    else if ((s[i] & 0xE0) == 0xC0){
      ch = s[i] & 0x1F;
      len = 2;
    }
    else if ((s[i] & 0xF0) == 0xE0){
      ch = s[i] & 0x0F;
      len = 3;
    }
    else if ((s[i] & 0xF8) == 0xF0){
      ch = s[i] & 0x07;
      len = 4;
    }
    else{
      ++i;
      continue;
    }
    size_t n = 1;
    while (n < len && i+n < length && (s[i+n] & 0xC0) == 0x80){
      ch = (ch << 6) | (s[i+n] & 0x3F);
      ++n;
    }
    if (n == len)
      character(ch);
    i += n;
  }
}

bool Keyboard::isPressed(int key) const{
  for (int pressed : pressedKeys_){
    if (pressed == key)
      return true;
  }
  return false;
}

