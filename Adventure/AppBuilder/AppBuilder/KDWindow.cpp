#include <KD/kd.h>
#include <system/engine.h>

#include "AdvDoc.h"
#include "Engine.h"
#include "Sound.h"
#include "Renderer.h"

EGLDisplay theDisplay;
EGLConfig theConfig;
EGLSurface theSurface;
EGLContext theContext;
KDWindow* theWindow;

AdvDocument* adoc;
static bool shouldQuit = false;

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
  
  Engine::instance()->initGame(quit);
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

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  GL()loadIdentity();

  Engine::instance()->render(20);

  SoundEngine::instance()->update();
}

KDint kdMain (KDint argc, const KDchar *const *argv){
  theDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (theDisplay == EGL_NO_DISPLAY){
    CGE::Log << "unable to get display" << std::endl;
  }
  if (!eglInitialize(theDisplay, NULL, NULL)){
    CGE::Log << "eglInitialize failed" << std::endl;
  }

  EGLint configAttribs[13];
  EGLint i = -1;
  configAttribs[++i] = EGL_RENDERABLE_TYPE;
  configAttribs[++i] = EGL_OPENGL_ES2_BIT;
  configAttribs[++i] = EGL_RED_SIZE;
  configAttribs[++i] = 8;
  configAttribs[++i] = EGL_GREEN_SIZE;
  configAttribs[++i] = 8;
  configAttribs[++i] = EGL_BLUE_SIZE;
  configAttribs[++i] = 8;
  configAttribs[++i] = EGL_ALPHA_SIZE;
  configAttribs[++i] = 8;
  configAttribs[++i] = EGL_DEPTH_SIZE;
  configAttribs[++i] = 24;
  configAttribs[++i] = EGL_NONE;

  EGLint numConfigs;
  if (!eglChooseConfig(theDisplay, configAttribs, &theConfig, 1, &numConfigs)){
    CGE::Log << "eglChooseConfig failed" << std::endl;
  }

  theWindow = kdCreateWindow(theDisplay, theConfig, KD_NULL);
  KDint32 winsize[] = {800, 480};
  kdSetWindowPropertyiv(theWindow, KD_WINDOWPROPERTY_SIZE, winsize);
  kdSetWindowPropertycv(theWindow, KD_WINDOWPROPERTY_CAPTION, "16");

  EGLNativeWindowType nativeWindow;
  if (kdRealizeWindow(theWindow, &nativeWindow))
    CGE::Log << "realizeWindow failed" << std::endl;

  theSurface = eglCreateWindowSurface(theDisplay, theConfig, nativeWindow, NULL);
  if (theSurface == EGL_NO_SURFACE)
    CGE::Log << "eglCreateWindowSurface failed" << std::endl;

  EGLint contextAttrs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
  theContext = eglCreateContext(theDisplay, theConfig, EGL_NO_CONTEXT, contextAttrs);

  eglMakeCurrent(theDisplay, theSurface, theSurface, theContext);

  std::string filename;
  if (argc > 1)
    filename = argv[1];
  else
    filename = "data/game.dat";
  initGame(filename);
  while(!shouldQuit){
    const KDEvent* ev = KD_NULL;
    while(ev = kdWaitEvent(0)){
    }
    render();
    eglSwapBuffers(theDisplay, theSurface);
  }

  Engine::instance()->exitGame();

  AdvRenderer::deinit();

  SoundEngine::deinit();
  Engine::deinit();
  delete adoc;

  eglMakeCurrent(theDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  eglDestroyContext(theDisplay, theContext);
  eglDestroySurface(theDisplay, theSurface);
  eglTerminate(theDisplay);

  kdDestroyWindow(theWindow);

  return 0;
}
