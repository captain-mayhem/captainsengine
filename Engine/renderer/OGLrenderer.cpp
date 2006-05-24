#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "../window/nativeWindows.h"
#include "../window/nativeLinux.h"
#include "../system/engine.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "OGLrenderer.h"

namespace Graphics{

OGLRenderer::OGLRenderer(): Renderer() {
  type_ = OpenGL;
#ifdef WIN32
  hDC_ = NULL;
  hRC_ = NULL;
#endif
#ifdef UNIX
  glx_ = NULL;
#endif
}

OGLRenderer::~OGLRenderer(){
}

void OGLRenderer::initContext(::Windows::AppWindow* win){
  ::System::Log << "Initializing OpenGL context\n";
  win_ = win;
#ifdef WIN32
  static PIXELFORMATDESCRIPTOR pfd ={
    sizeof(PIXELFORMATDESCRIPTOR),
      1,
      PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
      PFD_TYPE_RGBA,
      win->getBpp(),
      0, 0, 0, 0, 0, 0,   //Color bits ignored
      0,					//No Alpha buffer
      0,					//shift bit ignored
      0,					//no accumulation buffer
      0, 0, 0, 0,			//accumulation bits ignored
      16,					//Z-buffer depth
      0,					//no stencil buffer
      0,					//no auxiliary buffer
      PFD_MAIN_PLANE,
      0,					//Reserved
      0, 0, 0				//Layer masks ignored
  };

  HWND wnd = dynamic_cast<::Windows::WindowsWindow*>(win)->getHandle();
  if(!(hDC_ = GetDC(wnd))){
    System::Log << "Can't create GL device context\n";
    EXIT();
  }

  GLuint pixelFormat;
  if(!(pixelFormat = ChoosePixelFormat(hDC_, &pfd))){
    System::Log << "Can't find a suitable PixelFormat\n";
    EXIT();
  }

  if (!SetPixelFormat(hDC_, pixelFormat, &pfd)){
    System::Log << "Can't set the PixelFormat\n";
    EXIT();
  }

  if (!(hRC_ = wglCreateContext(hDC_))){
    System::Log << "Can't create GL rendering context\n";
    EXIT();
  }

  if (!wglMakeCurrent(hDC_, hRC_)){
    System::Log << "Cant't activate GL rendering context\n";
    EXIT();
  }

  ShowWindow(wnd, SW_SHOW);
  SetForegroundWindow(wnd);
  SetFocus(wnd);
#endif
#ifdef UNIX
  ::Windows::X11Window* x11 = dynamic_cast< ::Windows::X11Window* >(win_);
  glXMakeCurrent(x11->getDisplay(), x11->getWindow(), glx_);
#endif
  resizeScene(win->getWidth(), win->getHeight());
  initRendering();  
  
}

void OGLRenderer::killContext(){
#ifdef WIN32
  if (hRC_){
    if (!wglMakeCurrent(NULL,NULL)){
      ::System::Log << "Release of GL context failed";
    }
    if (!wglDeleteContext(hRC_)){
      ::System::Log << "Release of rendering context failed";
    }
    hRC_ = NULL;
  }

  HWND wnd = dynamic_cast<::Windows::WindowsWindow*>(win_)->getHandle();
  if (hDC_ && !ReleaseDC(wnd,hDC_)){
    ::System::Log << "Release of device context failed";
    hDC_ = NULL;
  }
#endif
#ifdef UNIX
  ::Windows::X11Window* x11 = dynamic_cast< ::Windows::X11Window* >(win_);
  if (glx_){
    if (!glXMakeCurrent(x11->getDisplay(), None, NULL)){
      ::System::Log << "Release of GL context failed";
    }
    glXDestroyContext(x11->getDisplay(), glx_);
    glx_ = NULL;
  }
#endif
}

void OGLRenderer::initRendering(){
  ::System::Log << "Initializing Scene\n";
  //smooth shading
  glShadeModel(GL_SMOOTH);

  //background color black
  //glClearColor(0.5f, 0.5f, 0.5f, 0.0f);

  //depth buffer
  glClearDepth(1.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  //better perspective calculations
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

  Renderer::initRendering();
}

void OGLRenderer::renderScene(){
  //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //glLoadIdentity();
  Renderer::renderScene();
}

void OGLRenderer::resizeScene(int width, int height){
#ifdef WIN32
  if (hRC_ == NULL)
    return;
#endif
  ::System::Log << "Resizing Scene\n";
  if (height == 0){
    height = 1;
  }
  win_->setWidth(width);
  win_->setHeight(height);

  glViewport(0, 0, width, height);

  //Projection Matrix
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  //Calculate Aspect Ratio
  gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,1.0f,100.0f);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

//! clear scene
void OGLRenderer::setClearColor(::Math::Vector3D color){
  glClearColor(color.x, color.y, color.z, 1.0);
}

void OGLRenderer::clear(long flags){
  long glflags = 0;
  if (flags & ZBUFFER)
    glflags |= GL_DEPTH_BUFFER_BIT;
  if (flags & COLORBUFFER)
    glflags |= GL_COLOR_BUFFER_BIT;
  glClear(glflags);
}

}
