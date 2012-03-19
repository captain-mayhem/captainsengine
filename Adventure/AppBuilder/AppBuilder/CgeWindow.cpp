#include "system/engine.h"
#include "renderer/renderer.h"
#include "system/utilities.h"
#include "input/mouse.h"
#include "input/keyboard.h"
#include "io/Tracing.h"
#include "io/TraceManager.h"

#include "AdvDoc.h"
#include "Engine.h"
#include "Sound.h"
#include "Renderer.h"
#include "CmdReceiver.h"

using namespace adv;

TR_CHANNEL(ADV_CGE_Window);

std::string filename;
AdvDocument* adoc = NULL;
CommandReceiver receiver;

void quit();

void init(){
  TR_USE(ADV_CGE_Window);
  adoc = new AdvDocument();
  if (!adoc->loadDocument(filename)){
    return;
  }
  Engine::init();
  Engine::instance()->setData(adoc);
  SoundEngine::init();
  SoundEngine::instance()->setData(adoc);

  GLenum err = glewInit();
  if (err != GLEW_OK){
    TR_ERROR("Unable to init OpenGL extensions");
    CGE::Engine::instance()->requestShutdown();
    return;
  }
  if (!GLEW_VERSION_2_0){
    TR_ERROR("OpenGL 2.0 not available");
    CGE::Engine::instance()->requestShutdown();
    return;
  }

  AdvRenderer::init();

  GL()matrixMode(MM_PROJECTION);
  GL()loadIdentity();
  GL()ortho(0, 640, 480, 0, -1.0, 1.0);
  //glFrustum(-0.5f, 0.5f, -0.5f, 0.5f, 1.0f, 3.0f);

  GL()matrixMode(MM_MODELVIEW);
  GL()color4ub(255,255,255,255);

  glDisable(GL_DEPTH_TEST);
  GL()enableClientState(ATTR_VERTEX_ARRAY);
  GL()enableClientState(ATTR_TEXTURE_COORD_ARRAY);
  GL()enable(GL_TEXTURE_2D);
  //glAlphaFunc(GL_GREATER, 0);
  //glEnable(GL_ALPHA_TEST);
  //glBlendFunc(GL_ZERO, GL_SRC_COLOR);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  glViewport(0, 0, 640, 480);
  
  receiver.start();
  Engine::instance()->initGame(quit);
}

void deinit(){
  Engine::instance()->exitGame();
  receiver.stop();

  AdvRenderer::deinit();

  SoundEngine::deinit();
  Engine::deinit();
  delete adoc;
}

void quit(){
  CGE::Engine::instance()->requestShutdown();
}

void render(){
  //CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  GL()matrixMode(MM_PROJECTION);
  GL()loadIdentity();
  GL()ortho(0, 640, 480, 0, -1.0, 1.0);
  //glFrustum(-0.5f, 0.5f, -0.5f, 0.5f, 1.0f, 3.0f);

  GL()matrixMode(MM_MODELVIEW);
  glDisable(GL_DEPTH_TEST);
  //glEnableClientState(GL_VERTEX_ARRAY);
  //glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  GL()enable(GL_TEXTURE_2D);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  glClearColor(0.0,0.0,0.0,1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  GL()loadIdentity();

  receiver.processCommands();
  unsigned time = (unsigned)(CGE::Engine::instance()->getFrameInterval()*1000);
  Engine::instance()->render(time);

  SoundEngine::instance()->update(time);
  CGE::Thread::sleep(5); //just a little sleep to avoid frame times < 1ms.
}

void mouse_move(int x, int y, int button){
  if (x >= 0 && x <= 640 && y >= 0 && y <= 480)
    Engine::instance()->setCursorPos(Vec2i(x,y));
}

void mouse_click(int x, int y, int button){
  Vec2i pos((int)(x/(float)SCREENWIDTH*640), (int)(y/(float)SCREENHEIGHT*480));
  if (button == MB_LEFT)
    Engine::instance()->leftClick(pos);
  else if (button == MB_RIGHT)
    Engine::instance()->rightClick(pos);
}

void mouse_release(int x, int y, int button){
  Vec2i pos((int)(x/(float)SCREENWIDTH*640), (int)(y/(float)SCREENHEIGHT*480));
  if (button == MB_LEFT)
    Engine::instance()->leftRelease(pos);
}

void double_click(int x, int y, int button){
  Vec2i pos((int)(x/(float)SCREENWIDTH*640), (int)(y/(float)SCREENHEIGHT*480));
  Engine::instance()->doubleClick(pos);
}

void mouse_wheel(int x, int y, int delta){
  Engine::instance()->mouseWheel(delta);
}

void key_press(int key){
  Engine::instance()->keyPress(key);
}

void key_release(int key){
  Engine::instance()->keyRelease(key);
}

void key_ascii(unsigned char ascii){
  Engine::instance()->keyAscii((char)ascii);
}

void engineMain(int argc, char** argv){
  CGE::LogOutputter* putty = new CGE::LogOutputter();
  CGE::TraceManager::instance()->setTraceOutputter(putty);
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
  Input::Keyboard::instance()->setAsciiCB(key_ascii);
  Input::Mouse::instance()->setButtonDownCB(mouse_click);
  Input::Mouse::instance()->setButtonUpCB(mouse_release);
  Input::Mouse::instance()->setMouseMoveCB(mouse_move);
  Input::Mouse::instance()->setDoubleClickCB(double_click);
  Input::Mouse::instance()->setWheelTurnCB(mouse_wheel);
  Input::Mouse::instance()->showCursor(false);
}
