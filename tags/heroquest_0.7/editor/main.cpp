#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <GL/gl.h>
//#include "include/cal3d/cal3d.h"
#include <iostream>
#include "system/engine.h"
#include "renderer/renderer.h"
#include "renderer/font.h"
#include "renderer/forms.h"
#include "math/vector.h"
#include "gui/messagebox.h"
#include "gui/dropdown.h"
#include "input/keyboard.h"
#include "input/mouse.h"

#include "graphics.h"
#include "editor.h"

using std::cerr;
using namespace Graphics;
using namespace System;

void engineMain(int argc, char** argv){
  Editor::instance()->init();
  
  //render callbacks
  Renderer* rend = Engine::instance()->getRenderer();
  rend->setRenderCB(Graphic::render);
  rend->setInitCB(Graphic::init);

  //input callbacks
  Input::Keyboard::instance()->setKeyDownCB(Editor::keypress);
  Input::Mouse::instance()->setButtonUpCB(Editor::mouseUp);
  Input::Mouse::instance()->setButtonDownCB(Editor::mouseDown);
}

