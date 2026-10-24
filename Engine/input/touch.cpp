#include "../system/engine.h"
#include "../input/mouse.h"
#include "../input/touch.h"
#include "../ui/imgui_impl_cge_platform.h"

using CGE::Vec2i;

using namespace Input;

Touch* Touch::touch_ = nullptr;

namespace{

//! window pixels in virtual coordinates (unchanged without an engine)
Vec2i toVirtual(int x, int y){
  CGE::Engine* engine = CGE::Engine::instance();
  return engine ? engine->getVirtualScreen().toVirtual(x, y) : Vec2i(x, y);
}

CGE::IApplication* application(){
  CGE::Engine* engine = CGE::Engine::instance();
  return engine ? engine->getApplication() : nullptr;
}

//! the mouse for the mouse emulation, while the engine has one
Mouse* emulatedMouse(){
  return CGE::Engine::instance() ? Mouse::instance() : nullptr;
}

}

Touch::Touch(){
  primary_ = -1;
  mouseEmulation_ = true;
  mousePressed_ = false;
}

void Touch::init(){
  touch_ = new Touch();
}

int Touch::find(int id) const{
  for (size_t i = 0; i < fingers_.size(); ++i){
    if (fingers_[i].id == id)
      return (int)i;
  }
  return -1;
}

bool Touch::isDown(int id) const{
  return find(id) >= 0;
}

void Touch::down(int id, int x, int y){
  //a finger the platform didn't lift first
  int index = find(id);
  if (index >= 0)
    remove(index, true);
  Finger finger;
  finger.id = id;
  finger.pos = toVirtual(x, y);
  finger.windowPos = Vec2i(x, y);
  //the first finger on an empty screen
  if (fingers_.empty()){
    primary_ = id;
    ImGui_ImplCGE_Platform_AddMouseSource(true);
    ImGui_ImplCGE_Platform_AddMousePos((float)x, (float)y);
    ImGui_ImplCGE_Platform_AddMouseButton(ImGuiMouseButton_Left, true);
    ImGui_ImplCGE_Platform_AddMouseSource(false);
  }
  fingers_.push_back(finger);
  if (CGE::IApplication* app = application())
    app->onTouchDown(id, finger.pos.x, finger.pos.y);
  Mouse* mouse = emulatedMouse();
  if (id == primary_ && mouseEmulation_ && mouse){
    mouse->move(x, y, 0);
    mouse->press(x, y, MB_LEFT);
    mousePressed_ = true;
  }
}

void Touch::move(int id, int x, int y){
  int index = find(id);
  if (index < 0 || fingers_[index].windowPos == Vec2i(x, y))
    return;
  Finger& finger = fingers_[index];
  finger.pos = toVirtual(x, y);
  finger.windowPos = Vec2i(x, y);
  Vec2i pos = finger.pos;
  if (id == primary_){
    ImGui_ImplCGE_Platform_AddMouseSource(true);
    ImGui_ImplCGE_Platform_AddMousePos((float)x, (float)y);
    ImGui_ImplCGE_Platform_AddMouseSource(false);
  }
  if (CGE::IApplication* app = application())
    app->onTouchMove(id, pos.x, pos.y);
  Mouse* mouse = emulatedMouse();
  if (id == primary_ && mousePressed_ && mouse)
    mouse->move(x, y, MB_LEFT);
}

void Touch::up(int id, int x, int y){
  int index = find(id);
  if (index < 0)
    return;
  //the last move may not have been reported
  move(id, x, y);
  remove(find(id), false);
}

void Touch::cancel(int id){
  int index = find(id);
  if (index >= 0)
    remove(index, true);
}

void Touch::cancelAll(){
  while (!fingers_.empty())
    remove((int)fingers_.size()-1, true);
}

void Touch::remove(int index, bool cancelled){
  Finger finger = fingers_[index];
  fingers_.erase(fingers_.begin()+index);
  bool primary = finger.id == primary_;
  if (primary){
    primary_ = -1;
    ImGui_ImplCGE_Platform_AddMouseSource(true);
    ImGui_ImplCGE_Platform_AddMousePos((float)finger.windowPos.x, (float)finger.windowPos.y);
    ImGui_ImplCGE_Platform_AddMouseButton(ImGuiMouseButton_Left, false);
    //the finger is gone, so ImGui shouldn't keep hovering there
    ImGui_ImplCGE_Platform_AddMouseLeave();
    ImGui_ImplCGE_Platform_AddMouseSource(false);
  }
  if (CGE::IApplication* app = application()){
    if (cancelled)
      app->onTouchCancel(finger.id);
    else
      app->onTouchUp(finger.id, finger.pos.x, finger.pos.y);
  }
  //the mouse has no cancel: its button is released either way
  Mouse* mouse = emulatedMouse();
  if (primary && mousePressed_ && mouse)
    mouse->buttonUp(finger.windowPos.x, finger.windowPos.y, MB_LEFT);
  if (primary)
    mousePressed_ = false;
}
