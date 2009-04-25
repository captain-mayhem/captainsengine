#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "../../window/nativeWindows.h"
#include "../../system/engine.h"
#include <GLES/gl.h>
#include "GLESvertexbuffer.h"
#include "GLEStexture.h"
#include "GLESrenderer.h"
#include "GLESindexbuffer.h"

using namespace CGE;

GLESRenderer::GLESRenderer(): Renderer() {
  type_ = OpenGL_ES;
}

GLESRenderer::~GLESRenderer(){
}

void GLESRenderer::initContext(::Windows::AppWindow* win){
  CGE::Log << "Initializing OpenGL ES context\n";
  win_ = win;
  EGLint rs = 0, gs = 0, bs = 0;
  if (win->getBpp() == 16){
    rs = 5; gs = 6; bs = 5;
  }
  else if (win->getBpp() == 32){
    rs = 8; gs = 8; bs = 8;
  }

  static const EGLint config[] = {
    EGL_RED_SIZE, rs,
    EGL_GREEN_SIZE, gs,
    EGL_BLUE_SIZE, bs,
    EGL_ALPHA_SIZE, 0,
    EGL_DEPTH_SIZE, 24,
    EGL_LUMINANCE_SIZE,			EGL_DONT_CARE,
    EGL_SURFACE_TYPE,			EGL_WINDOW_BIT,
    EGL_RENDERABLE_TYPE,		EGL_OPENGL_ES_BIT,
    EGL_BIND_TO_TEXTURE_RGBA,	EGL_DONT_CARE,
    EGL_NONE
  };
  display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (display_ == NULL){
    EXIT2("Cannot get display");
  }
  
  EGLint major, minor;
  if (!eglInitialize(display_, &major, &minor)){
    EXIT2("EGL initialization failed.");
  }
  else{
    CGE::Log << "EGL version " << major << "." << minor << " initalized.";
  }

  EGLint numConfigs;
  eglGetConfigs(display_, NULL, 0, &numConfigs);

  EGLConfig*	allConfigs;
  allConfigs = new EGLConfig[numConfigs];
  eglChooseConfig(display_, config, allConfigs, numConfigs, &numConfigs);

  static const EGLint surface[] =
  {
    EGL_COLORSPACE,		EGL_COLORSPACE_sRGB,
    EGL_ALPHA_FORMAT,	EGL_ALPHA_FORMAT_NONPRE,
    EGL_NONE
  };
  Windows::WindowsWindow* wnd = dynamic_cast<Windows::WindowsWindow*>(win);
  surface_ = eglCreateWindowSurface(display_, allConfigs[0], wnd->getHandle(), surface);
  
  eglBindAPI(EGL_OPENGL_ES_API);
  
  context_ = eglCreateContext(display_, allConfigs[0], NULL, NULL);
  if (context_ == NULL){
    EXIT2("Cannot create EGL context");
  }
  delete [] allConfigs;

  ShowWindow(wnd->getHandle(), SW_SHOW);
  SetForegroundWindow(wnd->getHandle());
  SetFocus(wnd->getHandle());

  eglMakeCurrent(display_, surface_, surface_, context_);

  resizeScene(win->getWidth(), win->getHeight());
}

void GLESRenderer::killContext(){
  eglMakeCurrent(display_,NULL, NULL, NULL);
  eglDestroyContext(display_, context_);
  eglDestroySurface(display_, surface_);
  eglTerminate(display_);
}

void GLESRenderer::initRendering(){
  CGE::Log << "Initializing Scene\n";
  //smooth shading
  glShadeModel(GL_SMOOTH);

  //depth buffer
  glClearDepthf(1.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  //better perspective calculations
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

  Renderer::initRendering();
}

void GLESRenderer::renderScene(){
  Renderer::renderScene();
}

void GLESRenderer::resizeScene(int width, int height){
  if (context_ == NULL)
    return;
  CGE::Log << "Resizing Scene\n";
  if (height == 0){
    height = 1;
  }
  win_->setWidth(width);
  win_->setHeight(height);

  glViewport(0, 0, width, height);

  Renderer::resizeScene(width, height);
}

//! clear scene
void GLESRenderer::setClearColor(::CGE::Vector3D color){
  glClearColor(color.x, color.y, color.z, 1.0);
}

void GLESRenderer::clear(long flags){
  long glflags = 0;
  if (flags & ZBUFFER)
    glflags |= GL_DEPTH_BUFFER_BIT;
  if (flags & COLORBUFFER)
    glflags |= GL_COLOR_BUFFER_BIT;
  glClear(glflags);
}

//! get a vertex buffer
VertexBuffer* GLESRenderer::createVertexBuffer(){
  return new GLESVertexBuffer();
}

IndexBuffer* GLESRenderer::createIndexBuffer(IndexBuffer::Type t, uint32 size){
  return new GLESIndexBuffer(t,size);
}

Texture* GLESRenderer::createTexture(string filename){
  return new GLESTexture(filename);
}

void GLESRenderer::lookAt(const Vector3D& position, const Vector3D& look, const Vector3D& up){
  Vec3f forward = look - position;
  forward.normalize();
  Vec3f side = forward.cross(up).normalized();
  Vec3f up_new = side.cross(forward);
  Matrix mat = Matrix(side, up_new, forward*-1, Vec3f()/*eye*1*/)*Matrix(Matrix::Translation,position*-1);
  multiplyMatrix(mat);
}

//! set projection
void GLESRenderer::projection(float angle, float aspect, float nearplane, float farplane){
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  GLfloat ymax = nearplane * (GLfloat)tan(angle * 3.1415962f / 360.0);
  GLfloat ymin = -ymax;
  GLfloat xmin = ymin * aspect;
  GLfloat xmax = ymax * aspect;
  glFrustumf(xmin, xmax, ymin, ymax, nearplane, farplane);
  glMatrixMode(GL_MODELVIEW);
}

void GLESRenderer::ortho(const int width, const int height){
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrthof(-width/2.0f, width/2.0f, -height/2.0f, height/2.0f, -1.0f, 1.0f);
  glMatrixMode(GL_MODELVIEW);
}

//! reset modelview matrix
void GLESRenderer::resetModelView(){
  glLoadIdentity();
}

//! translate
void GLESRenderer::translate(float x, float y, float z){
  glTranslatef(x,y,z);
}

//! scale
void GLESRenderer::scale(float x, float y, float z){
  glScalef(x,y,z);
}

//! set rendermode
void GLESRenderer::renderMode(RendMode rm){
  //TODO
  /*if (rm == Filled){
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
  else if(rm == Wireframe){
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }*/
}

//! set blending mode
void GLESRenderer::blendFunc(BlendType src, BlendType dest){
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
void GLESRenderer::enableBlend(const bool flag){
  if (flag)
    glEnable(GL_BLEND);
  else
    glDisable(GL_BLEND);
}

void GLESRenderer::enableBackFaceCulling(const bool flag){
  if (flag)
    glEnable(GL_CULL_FACE);
  else
    glDisable(GL_CULL_FACE);
}

//! enable texturing
void GLESRenderer::enableTexturing(const bool flag){
  if (flag)
    glEnable(GL_TEXTURE_2D);
  else
    glDisable(GL_TEXTURE_2D);
}

// enable lighting
void GLESRenderer::enableLighting(const bool flag){
  if (flag)
    glEnable(GL_LIGHTING);
  else
    glDisable(GL_LIGHTING);
}

//! set color
void GLESRenderer::setColor(float r, float g, float b, float a){
  glColor4f(r,g,b,a);
}

//! set color
void GLESRenderer::setColor(const Color* c){
  glColor4f(c->r, c->g, c->b, c->a);
}

//! push matrix
void GLESRenderer::pushMatrix(){
  glPushMatrix();
}

//! pop matrix
void GLESRenderer::popMatrix(){
  glPopMatrix();
}

//! multiply matrix
void GLESRenderer::multiplyMatrix(const CGE::Matrix& mat){
  glMultMatrixf(mat.getData());
}

//! set material
void GLESRenderer::setMaterial(const Material& mat){
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat.getAmbient().array);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat.getDiffuse().array);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat.getSpecular().array);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat.getEmissive().array);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &mat.getPower());
}

//! get the viewport
void GLESRenderer::getViewport(int view[4]){
  glGetIntegerv(GL_VIEWPORT, view);
}

//! get a matrix
Matrix GLESRenderer::getMatrix(MatrixType mt){
  float tmp[16];
  if (mt == Projection){
    glGetFloatv(GL_PROJECTION_MATRIX, tmp);
    return Matrix(tmp);
  }
  return Matrix(Matrix::Identity);
}

void GLESRenderer::swapBuffers(){
  eglSwapBuffers(display_, surface_);
}


