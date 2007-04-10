
#include "editor.h"
#include "graphics.h"

#include "math/arcball.h"
#include "input/keyboard.h"
#include "input/mouse.h"

using Math::Vector2D;

Editor* Editor::edi_ = NULL;

Editor::Editor(){
  arcball_ = new Math::Arcball();
}

Editor::~Editor(){
  SAFE_DELETE(arcball_);
}

void Editor::init(){
  edi_ = new Editor();
}

void Editor::keypress(int key){
  edi_->_keypress(key);
}

void Editor::_keypress(int key){
  if (key == KEY_ESCAPE)
    EXIT();
  if (key == KEY_W)
    std::cerr << "Test";
}

void Editor::update(){
  bool active = Input::Mouse::instance()->isPressed(MB_LEFT);
  bool reset = Input::Mouse::instance()->isPressed(MB_RIGHT);
  Vector2D pos = Input::Mouse::instance()->getMousePos();
  arcball_->update(active,reset,pos);
  Graphic::instance()->setTrafo(0, arcball_->getTrafo());
}

