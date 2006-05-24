#ifndef RENDERER_H
#define RENDERER_H

namespace Windows{
class AppWindow;
}

#include "../math/vector.h"

#define ZBUFFER 0x00000001
#define COLORBUFFER 0x00000010

namespace Renderer{

enum RenderType{
  OpenGL,
  DirectX
};

//! The abstract renderer
class Renderer{
public:
  Renderer();
  virtual ~Renderer();
  //! Returns the type of the renderer
  /*! It can be an OpenGL or DirectX renderer
  */
  RenderType getRenderType() {return type_;}
  //! initialize rendering context
  virtual void initContext(::Windows::AppWindow* win);
  //! kill rendering context
  virtual void killContext();
  //! initialize render settings
  virtual void initRendering();
  //! render scene
  virtual void renderScene();
  //! resize scene
  virtual void resizeScene(int width, int height);
  //! set clear color
  virtual void setClearColor(::Math::Vector3D color){}
  //! clear scene
  virtual void clear(long flags){}
protected:
  RenderType type_;
  ::Windows::AppWindow* win_;
};

}

#endif

