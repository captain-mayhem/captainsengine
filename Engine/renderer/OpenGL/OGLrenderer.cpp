#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "../../window/nativeWindows.h"
#include "../../window/nativeLinux.h"
#include "../../system/engine.h"
#include <GL/gl.h>
#include "OGLvertexbuffer.h"
#include "OGLtexture.h"
#include "OGLrenderer.h"
#include "OGLindexbuffer.h"

using namespace CGE;

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
#if defined(WIN32)
  wglDeleteContext(hRC_);
#endif
}

void OGLRenderer::initContext(::Windows::AppWindow* win){
  ::CGE::Log << "Initializing OpenGL context\n";
  win_ = win;
#if defined(WIN32) && !defined(UNDER_CE)
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

  HWND wnd = (HWND)dynamic_cast<::Windows::WindowsWindow*>(win)->getHandle();
  if(!(hDC_ = GetDC(wnd))){
    CGE::Log << "Can't create GL device context\n";
    EXIT();
  }

  GLuint pixelFormat;
  if(!(pixelFormat = ChoosePixelFormat(hDC_, &pfd))){
    CGE::Log << "Can't find a suitable PixelFormat\n";
    EXIT();
  }

  if (!SetPixelFormat(hDC_, pixelFormat, &pfd)){
    CGE::Log << "Can't set the PixelFormat\n";
    EXIT();
  }

  if (!(hRC_ = wglCreateContext(hDC_))){
    CGE::Log << "Can't create GL rendering context\n";
    EXIT();
  }

  if (!wglMakeCurrent(hDC_, hRC_)){
    CGE::Log << "Cant't activate GL rendering context\n";
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
#ifdef WIN32
  resizeScene(win->getWidth(), win->getHeight());
#endif
  //initRendering();  
  
}

void OGLRenderer::killContext(){
#if defined(WIN32) && !defined(UNDER_CE)
  if (hRC_){
    if (!wglMakeCurrent(NULL,NULL)){
      CGE::Log << "Release of GL context failed";
    }
    if (!wglDeleteContext(hRC_)){
      CGE::Log << "Release of rendering context failed";
    }
    hRC_ = NULL;
  }

  HWND wnd = (HWND)dynamic_cast<::Windows::WindowsWindow*>(win_)->getHandle();
  if (hDC_ && !ReleaseDC(wnd,hDC_)){
    CGE::Log << "Release of device context failed";
    hDC_ = NULL;
  }
#endif
#ifdef UNIX
  ::Windows::X11Window* x11 = dynamic_cast< ::Windows::X11Window* >(win_);
  if (glx_){
    if (!glXMakeCurrent(x11->getDisplay(), None, NULL)){
      CGE::Log << "Release of GL context failed";
    }
    glXDestroyContext(x11->getDisplay(), glx_);
    glx_ = NULL;
  }
#endif
}

void OGLRenderer::initRendering(){
  CGE::Log << "Initializing Scene\n";
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
  CGE::Log << "Resizing Scene\n";
  if (height == 0){
    height = 1;
  }
  win_->setWidth(width);
  win_->setHeight(height);

  glViewport(0, 0, width, height);

  Renderer::resizeScene(width, height);

  //Projection Matrix
  //glMatrixMode(GL_PROJECTION);
  //glLoadIdentity();

  //Calculate Aspect Ratio
  //gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,1.0f,100.0f);

  //glMatrixMode(GL_MODELVIEW);
  //glLoadIdentity();

  //glColor3f(0,0,0);
}

//! clear scene
void OGLRenderer::setClearColor(::CGE::Vector3D color){
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

//! get a vertex buffer
VertexBuffer* OGLRenderer::createVertexBuffer(){
  return new OGLVertexBuffer();
}

Texture* OGLRenderer::createTexture(string filename){
  return new OGLTexture(filename);
}

IndexBuffer* OGLRenderer::createIndexBuffer(IndexBuffer::Type t, uint32 size){
  return new OGLIndexBuffer(t,size);
}

void OGLRenderer::lookAt(const Vector3D& position, const Vector3D& look, const Vector3D& up){
  Vec3f forward = look - position;
  forward.normalize();
  Vec3f side = forward.cross(up).normalized();
  Vec3f up_new = side.cross(forward);
  Matrix mat = Matrix(side, up_new, forward*-1, Vec3f()/*eye*1*/)*Matrix(Matrix::Translation,position*-1);
  multiplyMatrix(mat);
}

//! set projection
void OGLRenderer::projection(float angle, float aspect, float nearplane, float farplane){
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  GLfloat ymax = nearplane * (GLfloat)tan(angle * 3.1415962f / 360.0);
  GLfloat ymin = -ymax;
  GLfloat xmin = ymin * aspect;
  GLfloat xmax = ymax * aspect;
  glFrustum(xmin, xmax, ymin, ymax, nearplane, farplane);
  glMatrixMode(GL_MODELVIEW);
}

void OGLRenderer::ortho(const int width, const int height){
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-width/2.0, width/2.0, -height/2.0, height/2.0, -1.f, 1.f);
  glMatrixMode(GL_MODELVIEW);
}

//! reset modelview matrix
void OGLRenderer::resetModelView(){
  glLoadIdentity();
}

//! translate
void OGLRenderer::translate(float x, float y, float z){
  glTranslatef(x,y,z);
}

//! scale
void OGLRenderer::scale(float x, float y, float z){
  glScalef(x,y,z);
}

//! set rendermode
void OGLRenderer::renderMode(RendMode rm){
  if (rm == Filled){
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
  else if(rm == Wireframe){
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }
}

//! set blending mode
void OGLRenderer::blendFunc(BlendType src, BlendType dest){
  int srcfl = 0;
  int destfl = 0;
  switch(src){
    case BLEND_ONE:
      srcfl = GL_ONE;
      break;
    case BLEND_SRC_ALPHA:
      srcfl = GL_SRC_ALPHA;
      break;
    default:
      break;
  }
  switch(dest){
    case BLEND_ONE:
      destfl = GL_ONE;
      break;
    case BLEND_ONE_MINUS_SRC_ALPHA:
      destfl = GL_ONE_MINUS_SRC_ALPHA;
      break;
    default:
      break;
  }
  glBlendFunc(srcfl, destfl);
}

//! enable blending
void OGLRenderer::enableBlend(const bool flag){
  if (flag)
    glEnable(GL_BLEND);
  else
    glDisable(GL_BLEND);
}

void OGLRenderer::enableBackFaceCulling(const bool flag){
  if (flag)
    glEnable(GL_CULL_FACE);
  else
    glDisable(GL_CULL_FACE);
}

//! enable texturing
void OGLRenderer::enableTexturing(const bool flag){
  if (flag)
    glEnable(GL_TEXTURE_2D);
  else
    glDisable(GL_TEXTURE_2D);
}

// enable lighting
void OGLRenderer::enableLighting(const bool flag){
  if (flag)
    glEnable(GL_LIGHTING);
  else
    glDisable(GL_LIGHTING);
}

//! set color
void OGLRenderer::setColor(float r, float g, float b, float a){
  glColor4f(r,g,b,a);
}

//! set color
void OGLRenderer::setColor(const Color* c){
  glColor4fv(c->array);
}

//! push matrix
void OGLRenderer::pushMatrix(){
  glPushMatrix();
}

//! pop matrix
void OGLRenderer::popMatrix(){
  glPopMatrix();
}

//! multiply matrix
void OGLRenderer::multiplyMatrix(const CGE::Matrix& mat){
  glMultMatrixf(mat.getData());
}

//! set material
void OGLRenderer::setMaterial(const Material& mat){
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat.getAmbient().array);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat.getDiffuse().array);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat.getSpecular().array);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat.getEmissive().array);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &mat.getPower());
}

//! get the viewport
void OGLRenderer::getViewport(int view[4]){
  glGetIntegerv(GL_VIEWPORT, view);
}

//! get a matrix
Matrix OGLRenderer::getMatrix(MatrixType mt){
  float tmp[16];
  if (mt == Modelview){
    glGetFloatv(GL_MODELVIEW_MATRIX, tmp);
    return Matrix(tmp);
  }
  if (mt == Projection){
    glGetFloatv(GL_PROJECTION_MATRIX, tmp);
    return Matrix(tmp);
  }
  return Matrix(Matrix::Identity);
}

void OGLRenderer::swapBuffers(){
#ifdef WIN32
  SwapBuffers(hDC_);
#endif
#ifdef UNIX
  Windows::X11Window* win = dynamic_cast<Windows::X11Window*>(win_);
  glXSwapBuffers(win->getDisplay(), win->getWindow());
#endif
}

void OGLRenderer::enableLight(short number, bool flag){
  if (flag)
    glEnable(GL_LIGHT0+number);
  else
    glDisable(GL_LIGHT0+number);
}

void OGLRenderer::setLight(int number, const Light& lit){
  Vec3f dir = lit.getDirection()*-1;
  float tmp[4];
  tmp[0] = dir.x;
  tmp[1] = dir.y;
  tmp[2] = dir.z;
  tmp[3] = 0;
  glLightfv(GL_LIGHT0, GL_POSITION, tmp);
}

void OGLRenderer::switchFromViewToModelTransform(){

}


