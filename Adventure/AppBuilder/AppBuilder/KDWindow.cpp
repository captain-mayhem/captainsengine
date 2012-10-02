#ifndef RASPBERRY_PI
#include <KD/kd.h>
#endif
#include <EGL/egl.h>
#include <system/engine.h>
#include <io/TraceManager.h>

#include "AdvDoc.h"
#include "Engine.h"
#include "Sound.h"
#include "Renderer.h"
#include "CmdReceiver.h"

using namespace adv;

TR_CHANNEL(ADV_KDWindow)

EGLDisplay theDisplay;
EGLConfig theConfig;
EGLSurface theSurface;
EGLContext theContext;
#ifndef RASPBERRY_PI
KDWindow* theWindow;
#else
EGL_DISPMANX_WINDOW_T theWindow;
#endif

AdvDocument* adoc;
static bool shouldQuit = false;
CommandReceiver receiver;

void quit(){
  shouldQuit = true;
}

void initGame(const std::string& filename){
  adoc = new AdvDocument();
  if (!adoc->loadDocument(filename)){
    return;
  }
  Engine::init();
  Engine::instance()->setData(adoc);
  SoundEngine::init();
  SoundEngine::instance()->setData(adoc);

  AdvRenderer::init();

  GL()matrixMode(MM_PROJECTION);
  GL()loadIdentity();
  GL()ortho(0, 640, 480, 0, -1.0, 1.0);
  //glFrustum(-0.5f, 0.5f, -0.5f, 0.5f, 1.0f, 3.0f);

  GL()matrixMode(MM_MODELVIEW);
  glClearColor(0.0,0.0,0.0,1.0);
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

void render(int time){
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

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  GL()loadIdentity();

  receiver.processCommands();
  Engine::instance()->render(time);

  SoundEngine::instance()->update(time);
}

#ifndef RASPBERRY_PI
KDint kdMain (KDint argc, const KDchar *const *argv){
#else
int main(int argc, char** argv){
#endif
  CGE::LogOutputter* putty = new CGE::LogOutputter();
  CGE::TraceManager::instance()->setTraceOutputter(putty);
  TR_USE(ADV_KDWindow);
#ifdef RASPBERRY_PI
  bcm_host_init();
#endif
  theDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (theDisplay == EGL_NO_DISPLAY){
    TR_ERROR("unable to get display");
  }
  if (!eglInitialize(theDisplay, NULL, NULL)){
    TR_ERROR("eglInitialize failed");
  }

  EGLint configAttribs[15];
  EGLint i = -1;
  configAttribs[++i] = EGL_RENDERABLE_TYPE;
  configAttribs[++i] = EGL_OPENGL_ES2_BIT;
  configAttribs[++i] = EGL_SURFACE_TYPE;
  configAttribs[++i] = EGL_WINDOW_BIT;
  configAttribs[++i] = EGL_RED_SIZE;
  configAttribs[++i] = 8;
  configAttribs[++i] = EGL_GREEN_SIZE;
  configAttribs[++i] = 8;
  configAttribs[++i] = EGL_BLUE_SIZE;
  configAttribs[++i] = 8;
  configAttribs[++i] = EGL_ALPHA_SIZE;
  configAttribs[++i] = 8;
  configAttribs[++i] = EGL_DEPTH_SIZE;
#ifdef WIN32
  configAttribs[++i] = 24;
#else
  configAttribs[++i] = 16;
#endif
  configAttribs[++i] = EGL_NONE;

  EGLint numConfigs;
  if (!eglChooseConfig(theDisplay, configAttribs, &theConfig, 1, &numConfigs)){
    TR_ERROR("eglChooseConfig failed");
  }

  eglBindAPI(EGL_OPENGL_ES_API);

#ifndef RASPBERRY_PI
  theWindow = kdCreateWindow(theDisplay, theConfig, KD_NULL);
  KDint32 winsize[] = {800, 480};
  kdSetWindowPropertyiv(theWindow, KD_WINDOWPROPERTY_SIZE, winsize);
  kdSetWindowPropertycv(theWindow, KD_WINDOWPROPERTY_CAPTION, "16");

  EGLNativeWindowType nativeWindow;
  if (kdRealizeWindow(theWindow, &nativeWindow))
    TR_ERROR("realizeWindow failed");
#else
  unsigned screen_width, screen_height;
  int success = graphics_get_display_size(0, &screen_width, &screen_height);
  VC_RECT_T dstRect, srcRect;
  dstRect.x = 0;
  dstRect.y = 0;
  dstRect.width = screen_width;
  dstRect.height = screen_height;
  srcRect.x = 0;
  srcRect.y = 0;
  srcRect.width = screen_width << 16;
  srcRect.height = screen_height << 16;
  DISPMANX_DISPLAY_HANDLE_T dispman_display = vc_dispmanx_display_open(0);
  DISPMANX_UPDATE_HANDLE_T dispman_update = vc_dispmanx_update_start(0);
  DISPMANX_ELEMENT_HANDLE_T dispman_element = vc_dispmanx_element_add(dispman_update, dispman_display, 0, &dstRect, 0, &srcRect, DISPMANX_PROTECTION_NONE,
      0, 0, (DISPMANX_TRANSFORM_T)0);
  theWindow.element = dispman_element;
  theWindow.width = screen_width;
  theWindow.height = screen_height;
  vc_dispmanx_update_submit_sync(dispman_update);
  EGL_DISPMANX_WINDOW_T* nativeWindow = &theWindow;
#endif
  theSurface = eglCreateWindowSurface(theDisplay, theConfig, nativeWindow, NULL);
  if (theSurface == EGL_NO_SURFACE)
    TR_ERROR("eglCreateWindowSurface failed");

  EGLint contextAttrs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
  theContext = eglCreateContext(theDisplay, theConfig, EGL_NO_CONTEXT, contextAttrs);

  eglMakeCurrent(theDisplay, theSurface, theSurface, theContext);

  std::string filename;
  if (argc > 1)
    filename = argv[1];
  else
    filename = "data/game.dat";
  initGame(filename);
#ifndef RASPBERRY_PI
  KDust lasttime;
  lasttime = kdGetTimeUST();
  KDtime newtime;
  kdSetTimer(10000000, KD_TIMER_PERIODIC_AVERAGE, NULL);
  while(!shouldQuit){
    const KDEvent* ev = KD_NULL;
    while(ev = kdWaitEvent(5)){
      switch(ev->type){
        case KD_EVENT_WINDOW_CLOSE:
          shouldQuit = true;
          break;
        case KD_EVENT_INPUT_POINTER:{
          Vec2i pos;
          pos.x = ev->data.inputpointer.x;
          pos.y = ev->data.inputpointer.y;
          if (ev->data.inputpointer.index == KD_INPUT_POINTER_X){
            Engine::instance()->setCursorPos(pos);
          }
          else if (ev->data.inputpointer.index == KD_INPUT_POINTER_SELECT){
            if (ev->data.inputpointer.select == 1)
              Engine::instance()->leftClick(pos);
          }
          else
            TR_ERROR("pointer: index %i select %i unhandled", ev->data.inputpointer.index, ev->data.inputpointer.select);
          break;
        }
        case KD_EVENT_WINDOW_FOCUS:
          kdSetWindowPropertyiv(theWindow, KD_WINDOWPROPERTY_FOCUS, &ev->data.windowfocus.focusstate);
          //kdSetWindowPropertyiv(theWindow, KD_WINDOWPROPERTY_VISIBILITY, &ev->data.windowfocus.focusstate);
          break;
        case KD_EVENT_WINDOW_REDRAW:
        case KD_EVENT_WINDOWPROPERTY_CHANGE:
          kdDefaultEvent(ev);
          break;
        case KD_EVENT_TIMER:
          newtime = kdGetTimeUST();
          render((int)((newtime-lasttime)/1000000));
          lasttime = newtime;
          eglSwapBuffers(theDisplay, theSurface);
          break;
        default:
          kdDefaultEvent(ev);
      }
    }
  }
#else
  while(!shouldQuit){
    render(10);
    eglSwapBuffers(theDisplay, theSurface);
  }
#endif
  Engine::instance()->exitGame();
  receiver.stop();

  AdvRenderer::deinit();

  SoundEngine::deinit();
  Engine::deinit();
  delete adoc;

  eglMakeCurrent(theDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  eglDestroyContext(theDisplay, theContext);
  eglDestroySurface(theDisplay, theSurface);
  eglTerminate(theDisplay);
#ifndef RASPBERRY_PI
  kdDestroyWindow(theWindow);
#endif
  return 0;
}
