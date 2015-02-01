#ifndef GL2RENDERER_H
#define GL2RENDERER_H

#ifndef OPENGL2
#define Dummy OGL
#include "../dummyrenderer.h"
#undef Dummy
#else
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#if defined UNIX && !defined QNX
#include <GL/glew.h>
#include <GL/glx.h>
#endif
#include "../renderer.h"
#include "GL2Shader.h"
#include "GL2rendertarget.h"

#include <stack>

namespace CGE{
//! The OpenGL version of the rendering interface
class GL2Renderer : public Renderer{
public: 
  //! Constructor
  GL2Renderer();
  //! Destructor
  ~GL2Renderer();
  //! init rendering context
  void initContext(AppWindow* win);
#ifdef WIN32
  //! get the device context
  inline HDC getDevice() {return hDC_;}
#endif
#if defined UNIX && !defined QNX
  //set glx context
  inline void setGLX(GLXContext glx){glx_ = glx;}
  //get glx context
  inline GLXContext getGLX() const {return glx_;}
#endif
  //! kill rendering context
  void killContext();
  //! initialize scene
  void initRendering();
  //! render scene
  void renderScene();
  //! resize scene
  void resizeScene(int width, int height);
  //! clear scene
  virtual void setClearColor(::CGE::Vector3D color);
  //! clear scene
  virtual void clear(long flags);
  //! get a vertex buffer
  VertexBuffer* createVertexBuffer();
  //! get a index buffer
  virtual IndexBuffer* createIndexBuffer(IndexBuffer::Type t, uint32 size);
  //! create texture
  Texture* createTexture(Image* img, Texture::Format fmt);
  //! create rendertarget
  virtual RenderTarget* createRenderTarget(unsigned width, unsigned height);
  //! set look at
  void lookAt(const CGE::Vec3f& position, const CGE::Vec3f& look, const CGE::Vec3f& up);
  //! set projection
  void projection(float angle, float aspect, float nearplane, float farplane);
  //! set orthographic viewing
  void ortho(float left, float right, float bottom, float top, float nearp, float farp);
  //! reset modelview matrix
  void resetModelView();
  //! translate
  void translate(float x, float y, float z);
  //! scale
  void scale(float x, float y, float z);
  //! rotate
  virtual void rotate(float angle, float x, float y, float z);
  //! set rendermode
  void renderMode(RendMode rm);
  //! set blending mode
  void blendFunc(BlendType src, BlendType dest);
  //! enable blending
  void enableBlend(const bool flag);
  //! enable backface culling
  void enableBackFaceCulling(const bool flag);
  //! enable texturing
  void enableTexturing(const bool flag);
  //! enable lighting
  void enableLighting(const bool flag);
  //! enable light
  virtual void enableLight(short number, bool flag);
  //! enable depth test
  virtual void enableDepthTest(const bool flag);
  //! set color
  void setColor(float r, float g, float b, float a);
  //! set color
  void setColor(const Color* c);
  //! set material
  //void setMaterial(const Material& mat);
  //! set light
  virtual void setLight(int number, const Light& lit);
  //! push matrix
  void pushMatrix();
  //! pop matrix
  void popMatrix();
  //! multiply matrix
  void multiplyMatrix(const CGE::Matrix& mat);
  //! get the viewport
  virtual void getViewport(int view[4]);
  //! get a matrix
  virtual CGE::Matrix getMatrix(MatrixType mt);
  //! swap back and front buffer
  virtual void swapBuffers();
  //! switch matrix stack
  virtual void switchMatrixStack(MatrixType type);
protected:
#ifdef WIN32
  //! device context
  HDC hDC_;
  //! GL rendering context
  HGLRC hRC_;
#endif
#if defined UNIX && !defined QNX
  GLXContext glx_;
#endif
  //! matrix stacks
  std::stack<CGE::Matrix> mMatrixStack[3];
  CGE::Matrix mMatrix[3];
  MatrixType mMatrixMode;
  //! shader
  CGE::GL2Shader* mShader;
  //! the framebuffer
  GL2RenderTarget* mRT;
};

}

#endif

#endif

