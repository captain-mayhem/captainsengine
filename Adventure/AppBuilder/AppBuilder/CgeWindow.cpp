#include "system/engine.h"
#include "renderer/renderer.h"
#include "system/utilities.h"
#include "input/mouse.h"
#include "input/keyboard.h"

#include <wx/app.h>
#include <wx/wfstream.h>
#include <wx/fs_arc.h>

#include "AdvDoc.h"
#include "Engine.h"
#include "Sound.h"

std::string filename;
AdvDocument* adoc = NULL;

void init(){
  wxInitialize();
  wxInitAllImageHandlers();
  wxFileSystem::AddHandler(new wxLocalFSHandler);
  wxFileSystem::AddHandler(new wxArchiveFSHandler);

  adoc = new AdvDocument();
  adoc->SetFilename(wxString::FromAscii(filename.c_str()));
  wxFileInputStream fis(wxString::FromAscii(filename.c_str()));
  adoc->LoadObject(fis);
  Engine::init();
  Engine::instance()->setData(adoc);
  SoundEngine::init();
  SoundEngine::instance()->setData(adoc);

  GLenum err = glewInit();
  if (err != GLEW_OK)
    assert(false && "Unable to init OpenGL extensions");
  if (!GLEW_VERSION_2_0)
    assert(false && "OpenGL 2.0 not available");
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, 640, 480, 0, -1.0, 1.0);
  //glFrustum(-0.5f, 0.5f, -0.5f, 0.5f, 1.0f, 3.0f);

  glMatrixMode(GL_MODELVIEW);
  glClearColor(0.0,0.0,0.0,1.0);
  glColor4ub(255,255,255,255);

  glDisable(GL_DEPTH_TEST);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnable(GL_TEXTURE_2D);
  //glAlphaFunc(GL_GREATER, 0);
  //glEnable(GL_ALPHA_TEST);
  //glBlendFunc(GL_ZERO, GL_SRC_COLOR);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  glViewport(0, 0, 640, 480);
  
  Engine::instance()->initGame();
}

void deinit(){
  Engine::instance()->exitGame();

  SoundEngine::deinit();
  Engine::deinit();
  delete adoc;

  wxFileSystem::CleanUpHandlers();
  wxUninitialize();
}

void render(){
  //CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, 640, 480, 0, -1.0, 1.0);
  //glFrustum(-0.5f, 0.5f, -0.5f, 0.5f, 1.0f, 3.0f);

  glMatrixMode(GL_MODELVIEW);
  glDisable(GL_DEPTH_TEST);
  //glEnableClientState(GL_VERTEX_ARRAY);
  //glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnable(GL_TEXTURE_2D);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();

  Engine::instance()->render((int)(CGE::Engine::instance()->getFrameInterval()*1000));

  SoundEngine::instance()->update();
}

void mouse_move(int x, int y, int button){
  Engine::instance()->setCursorPos(Vec2i(x,y));
}

void mouse_click(int x, int y, int button){
  Vec2i pos(x/(float)SCREENWIDTH*640, y/(float)SCREENHEIGHT*480);
  if (button == MB_LEFT)
    Engine::instance()->leftClick(pos);
  else if (button == MB_RIGHT)
    Engine::instance()->rightClick(pos);
}

void key_press(int key){
  Engine::instance()->keyPress(key);
}

void key_release(int key){
  Engine::instance()->keyRelease(key);
}

void engineMain(int argc, char** argv){
  if (argc > 1)
    filename = argv[1];
  else
    filename = "data/game.dat";
  CGE::Utilities::replaceWith(filename, '\\', '/');
  //render callbacks
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  rend->setRenderCB(render);
  rend->setInitCB(init);
  rend->setDeinitCB(deinit);

  //input callbacks
  Input::Keyboard::instance()->setKeyDownCB(key_press);
  Input::Keyboard::instance()->setKeyUpCB(key_release);
  Input::Mouse::instance()->setButtonDownCB(mouse_click);
  Input::Mouse::instance()->setMouseMoveCB(mouse_move);
  Input::Mouse::instance()->showCursor(false);
}
