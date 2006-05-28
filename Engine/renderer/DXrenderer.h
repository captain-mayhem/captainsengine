#ifndef DXRENDERER_H
#define DXRENDERER_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d9.h>
#include "../math/vector.h"
#include "renderer.h"

namespace Graphics{

class VertexBuffer;

//! The DirectX version of the rendering interface
class DXRenderer : public Renderer{
public: 
  //! Constructor
  DXRenderer();
  //! Destructor
  ~DXRenderer();
  //! get device
  inline LPDIRECT3DDEVICE9 getDevice() const {return device_;}
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
  //! get a vertex buffer
  VertexBuffer* createVertexBuffer();
  //! get a texture
  Texture* createTexture(string filename);
  //! set lookAt
  void lookAt(const Vector3D* position, const Vector3D* look, const Vector3D* up);
  //! set perspective projection
  void projection(float angle, float aspect, float nearplane, float farplane);
  //! set orthographic projection
  void ortho();
  //! reset modelview matrix
  void resetModelView();
  //! translate
  void translate(float x, float y, float z);
  //! set render mode
  void renderMode(RendMode rm);
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
