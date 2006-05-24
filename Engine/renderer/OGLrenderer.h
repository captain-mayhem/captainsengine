#ifndef OGLRENDERER_H
#define OGLRENDERER_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#ifdef UNIX
#include <GL/glx.h>
#endif
#include "renderer.h"

namespace Renderer{
//! The OpenGL version of the rendering interface
class OGLRenderer : public Renderer{
public: 
  //! Constructor
  OGLRenderer();
  //! Destructor
  ~OGLRenderer();
  //! init rendering context
  void initContext(::Windows::AppWindow* win);
#ifdef WIN32
  //! get the device context
  inline HDC getDevice() {return hDC_;}
#endif
#ifdef UNIX
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
  virtual void setClearColor(::Math::Vector3D color);
  //! clear scene
  virtual void clear(long flags);
protected:
#ifdef WIN32
  //! device context
  HDC hDC_;
  //! GL rendering context
  HGLRC hRC_;
#endif
#ifdef UNIX
  GLXContext glx_;
#endif
};

}

#endif

