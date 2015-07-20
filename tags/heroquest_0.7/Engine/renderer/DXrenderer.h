#ifndef DXRENDERER_H
#define DXRENDERER_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <stack>
#include "../math/vector.h"
#include "renderer.h"

using std::stack;

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
  void lookAt(const Vector3D& position, const Vector3D& look, const Vector3D& up);
  //! set perspective projection
  void projection(float angle, float aspect, float nearplane, float farplane);
  //! set orthographic projection
  void ortho(const int width, const int height);
  //! reset modelview matrix
  void resetModelView();
  //! translate
  void translate(float x, float y, float z);
  //! scale
  void scale(float x, float y, float z);
  //! set render mode
  void renderMode(RendMode rm);
  //! set blending mode
  void blendFunc(BlendType src, BlendType dest);
  //! enable blending
  void enableBlend(const bool flag);
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
protected:
  //! the directX context
  LPDIRECT3D9 d3d_;
  //! the graphics device
  LPDIRECT3DDEVICE9 device_;
  //! the clear color
  ::Math::Vector3D clearColor_;
  //! the matrix stack
  stack<D3DXMATRIX> modelstack_;
};

}

#endif

#ifdef UNIX
#include "renderer.h"

namespace Graphics{
class DXRenderer : public Renderer{
public: 
  //! Constructor
  DXRenderer(){}
  //! Destructor
  ~DXRenderer(){}  
  //! create texture
  Texture* createTexture(string filename){return NULL;}
  //! set orthographic projection
  void ortho(const int width, const int height){}
  //! reset modelview matrix
  void resetModelView(){}
  //! translate
  void translate(float x, float y, float z){}
  //! scale
  void scale(float x, float y, float z){}
  //! set render mode
  void renderMode(RendMode rm){}
  //! set blending mode
  void blendFunc(BlendType src, BlendType dest){}
  //! enable blending
  void enableBlend(const bool flag){}
  //! enable texturing
  void enableTexturing(const bool flag){}
  //! enable lighting
  void enableLighting(const bool flag){}
  //! set color
  void setColor(float r, float g, float b, float a){}
  //! set color
  void setColor(const Color* c){}
  //! set material
  void setMaterial(const Material& mat){}
  //! push matrix
  void pushMatrix(){}
  //! pop matrix
  void popMatrix(){}
};
}
#endif


#endif
