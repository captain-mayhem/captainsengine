#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
//#include <GL/gl.h>
//#include "include/cal3d/cal3d.h"
#include <iostream>
#include "system/engine.h"
#include "system/application.h"
#include "renderer/renderer.h"
#include "renderer/font.h"
#include "renderer/forms.h"
#include "math/vector.h"
#include "input/keyboard.h"
#include "input/mouse.h"
#include "io/TraceManager.h"

#include "graphics.h"
#include "editor.h"

using std::cerr;
using namespace CGE;

class EditorApp : public CGE::IApplication{
public:
  void init(int argc, char** argv) override;
  void render() override {Graphic::render();}
  void onResize(int width, int height) override {Graphic::resize(width, height);}
  void onKeyDown(int key) override {Editor::keypress(key);}
  void onMouseDown(int x, int y, int button) override {Editor::mouseDown(x, y, button);}
  void onMouseUp(int x, int y, int button) override {Editor::mouseUp(x, y, button);}
};

void EditorApp::init(int argc, char** argv){
  CGE::LogOutputter* putty = new CGE::LogOutputter("editor.log");
  CGE::TraceManager::instance()->setTraceOutputter(putty);
  Editor::init();

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

  Graphic::init();
}

CGE_MAIN(EditorApp)
