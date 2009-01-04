#ifndef GLESRENDERER_H
#define GLESRENDERER_H

#ifndef GLES
#define Dummy GLES
#include "../dummyrenderer.h"
#undef Dummy
#else
#include <EGL/egl.h>
#include "../renderer.h"

namespace Graphics{
//! The OpenGL ES version of the rendering interface
class GLESRenderer : public Renderer{
public: 
  //! Constructor
  GLESRenderer();
  //! Destructor
  ~GLESRenderer();
  //! init rendering context
  void initContext(::Windows::AppWindow* win);
  //! kill rendering context
  void killContext();
  //! initialize scene
  void initRendering();
  //! render scene
  void renderScene();
  //! resize scene
  void resizeScene(int width, int height);
  //! clear scene
  virtual void setClearColor(::Math::Vector3D color);
  //! clear scene
  virtual void clear(long flags);
  //! get a vertex buffer
  VertexBuffer* createVertexBuffer();
  //! create texture
  Texture* createTexture(string filename);
  //! set look at
  void lookAt(const Vector3D& position, const Vector3D& look, const Vector3D& up);
  //! set projection
  void projection(float angle, float aspect, float nearplane, float farplane);
  //! set orthographic viewing
  void ortho(const int width, const int height);
  //! reset modelview matrix
  void resetModelView();
  //! translate
  void translate(float x, float y, float z);
  //! scale
  void scale(float x, float y, float z);
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
  //! set color
  void setColor(float r, float g, float b, float a);
  //! set color
  void setColor(const Color* c);
  //! set material
  void setMaterial(const Material& mat);
  //! push matrix
  void pushMatrix();
  //! pop matrix
  void popMatrix();
  //! multiply matrix
  void multiplyMatrix(const Math::Matrix& mat);
  //! get the viewport
  virtual void getViewport(int view[4]);
  //! get a matrix
  virtual Matrix getMatrix(MatrixType mt);
  //! swap back and front buffer
  virtual void swapBuffers();
protected:
  EGLDisplay display_;
  EGLContext context_;
  EGLSurface surface_;
};

}

#endif

#endif

