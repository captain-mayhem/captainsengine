#ifndef RENDERER_H
#define RENDERER_H

namespace Windows{
class AppWindow;
}

#include "../math/vector.h"
#include "../system/interactor.h"
#include "vertexbuffer.h"
#include "texture.h"

using namespace ::Math;

#define ZBUFFER 0x00000001
#define COLORBUFFER 0x00000010

namespace Graphics{

enum RenderType{
  OpenGL,
  DirectX
};

enum BlendType{
  BLEND_ONE,
  BLEND_SRC_ALPHA
};

enum RendMode{
  Filled,
  Wireframe
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
  //! set render callback
  void setRenderCB(void (*proc)()) {renderCB_ = proc;}
  //! set init callback
  void setInitCB(void (*proc)()) {initCB_ = proc;}
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
  virtual void setClearColor(Vector3D color){}
  //! clear scene
  virtual void clear(long flags){}
  //! create vertex buffer
  virtual VertexBuffer* createVertexBuffer(){return NULL;}
  //! create texture
  virtual Texture* createTexture(string filename)=0;
  //! set lookAt
  virtual void lookAt(const Vector3D* position, const Vector3D* look, const Vector3D* up){}
  //! set perspective projection
  virtual void projection(float angle, float aspect, float nearplane, float farplane){}
  //! set orthographic projection
  virtual void ortho(const int width, const int height)=0;
  //! reset modelview matrix
  virtual void resetModelView()=0;
  //! translate
  virtual void translate(float x, float y, float z)=0;
  //! set render mode
  virtual void renderMode(RendMode rm){}
  //! set blending mode
  virtual void blendFunc(BlendType src, BlendType dest)=0;
  //! enable blending
  virtual void enableBlend(const bool flag)=0;
  //! set color
  virtual void setColor(char r, char g, char b, char a)=0;
protected:
  //! the type of the renderer
  /*! can be OpenGL or DirectX*/
  RenderType type_;
  //! the window in which is rendered
  ::Windows::AppWindow* win_;
  //! the render callback
  void (*renderCB_)();
  //! the init callback
  void (*initCB_)();
};

}

#endif

