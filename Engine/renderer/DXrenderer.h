#ifndef DXRENDERER_H
#define DXRENDERER_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d9.h>
#include "../math/vector.h"
#include "renderer.h"

namespace Renderer{
//! The DirectX version of the rendering interface
class DXRenderer : public Renderer{
public: 
  //! Constructor
  DXRenderer();
  //! Destructor
  ~DXRenderer();
  //! init rendering context
  void initContext(::Windows::AppWindow* win);
  //! kill rendering context
  void killContext();
  //! start scene
  void initRendering();
  //! render scene
  void renderScene();
  //! resize event
  void resizeScene(int width, int height);
  //! set clear color
  void setClearColor(::Math::Vector3D color);
  //! clear scene
  void clear(long flags);
protected:
  //! the directX context
  LPDIRECT3D9 d3d_;
  //! the graphics device
  LPDIRECT3DDEVICE9 device_;
  //! the clear color
  ::Math::Vector3D clearColor_;
};

}

#endif

#ifdef UNIX
namespace Renderer{
class DXRenderer : public Renderer{
public: 
  //! Constructor
  DXRenderer(){}
  //! Destructor
  ~DXRenderer(){}
};
}
#endif


#endif
