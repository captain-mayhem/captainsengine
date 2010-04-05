#ifndef RENDERER_H
#define RENDERER_H

namespace Windows{
class AppWindow;
}

#include "../math/vector.h"
#include "../math/matrix.h"
#include "vertexbuffer.h"
#include "indexbuffer.h"
#include "texture.h"
#include "light.h"

#define ZBUFFER 0x00000001
#define COLORBUFFER 0x00000010

namespace CGE{

enum RenderType{
  OpenGL,
  DirectX,
  OpenGL_ES
};

enum BlendType{
  BLEND_ONE,
  BLEND_SRC_ALPHA,
  BLEND_ONE_MINUS_SRC_ALPHA
};

enum RendMode{
  Filled,
  Wireframe
};

enum MatrixType{
  Projection,
  Modelview,
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
  //! set deinit callback
  void setDeinitCB(void (*proc)()) {deinitCB_ = proc;}
  //! set resize callback
  void setResizeCB(void (*proc)(int width, int height)) {resizeCB_ = proc;}
  //! initialize rendering context
  virtual void initContext(::Windows::AppWindow* win);
  //! kill rendering context
  virtual void killContext();
  //! initialize render settings
  virtual void initRendering();
  //! deinitialize render settings
  virtual void deinitRendering();
  //! render scene
  virtual void renderScene();
  //! resize scene
  virtual void resizeScene(int width, int height);
  //! set clear color
  virtual void setClearColor(CGE::Vec3f color){}
  //! clear scene
  virtual void clear(long flags){}
  //! create vertex buffer
  virtual VertexBuffer* createVertexBuffer(){return NULL;}
  //! create index buffer
  virtual IndexBuffer* createIndexBuffer(IndexBuffer::Type t, uint32 size){return NULL;}
  //! create texture
  virtual Texture* createTexture(string filename)=0;
  //! set lookAt
  virtual void lookAt(const CGE::Vec3f& position, const CGE::Vec3f& look, const CGE::Vec3f& up){}
  //! set perspective projection
  virtual void projection(float angle, float aspect, float nearplane, float farplane){}
  //! set orthographic projection
  virtual void ortho(const int width, const int height)=0;
  //! reset modelview matrix
  virtual void resetModelView()=0;
  //! translate
  virtual void translate(float x, float y, float z)=0;
  //! scale
  virtual void scale(float x, float y, float z)=0;
  //! set render mode
  virtual void renderMode(RendMode rm){}
  //! set blending mode
  virtual void blendFunc(BlendType src, BlendType dest)=0;
  //! enable blending
  virtual void enableBlend(const bool flag)=0;
  //! enable culling
  virtual void enableBackFaceCulling(const bool flag)=0;
  //! enable texturing
  virtual void enableTexturing(const bool flag)=0;
  //! enable lighting
  virtual void enableLighting(const bool flag)=0;
  //! enable light
  virtual void enableLight(short number, bool flag)=0;
  //! set color
  virtual void setColor(float r, float g, float b, float a)=0;
  //! set color
  virtual void setColor(const Color* c)=0;
  //! set material
  virtual void setMaterial(const Material& mat)=0;
  //! set light
  virtual void setLight(int number, const Light& lit)=0;
  //! push matrix
  virtual void pushMatrix()=0;
  //! pop matrix
  virtual void popMatrix()=0;
  //! multiply matrix
  virtual void multiplyMatrix(const CGE::Matrix& mat)=0;
  //! get the viewport
  virtual void getViewport(int view[4])=0;
  //! get a matrix
  virtual CGE::Matrix getMatrix(MatrixType mt)=0;
  //! swap back and front buffer
  virtual void swapBuffers()=0;
  //! switch from view to model matrix
  virtual void switchFromViewToModelTransform()=0;
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
  //! the deinit callback
  void (*deinitCB_)();
  //! the resize callback
  void (*resizeCB_)(int width, int height);
};

}

#endif

