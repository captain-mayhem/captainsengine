#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
//#include <GL/gl.h>
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
#include "io/TraceManager.h"

#include "graphics.h"
#include "editor.h"

using std::cerr;
using namespace CGE;

void engineMain(int argc, char** argv){
  CGE::LogOutputter* putty = new CGE::LogOutputter("editor.log");
  CGE::TraceManager::instance()->setTraceOutputter(putty);
  Editor::instance()->init();
  
  //render callbacks
  Renderer* rend = Engine::instance()->getRenderer();
  rend->setRenderCB(Graphic::render);
  rend->setInitCB(Graphic::init);

  //input callbacks
  Input::Keyboard::instance()->setKeyDownCB(Editor::keypress);
  Input::Mouse::instance()->setButtonUpCB(Editor::mouseUp);
  Input::Mouse::instance()->setButtonDownCB(Editor::mouseDown);

  //check command line
  int state = 0;
  for (int i = 1; i < argc; ++i){
    if (state == 0){
      if (strcmp(argv[i], "-m") == 0){
        state = 1;
      }
      else if (strcmp(argv[i], "-s") == 0){
        state = 2;
      }
    }
    else if (state == 1){
      char* model = argv[i];
      Editor::instance()->addStartupModel(model);
      state = 0;
    }
    else if (state == 2){
      char* scene = argv[i];
      Editor::instance()->setStartupScene(scene);
      state = 0;
    }
  }
}

