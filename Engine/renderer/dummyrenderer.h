#ifndef DUMMYRENDERER_H
#define DUMMYRENDERER_H

#include "renderer.h"

namespace CGE{
class DummyRenderer : public Renderer{
public: 
  //! Constructor
  DummyRenderer(){}
  //! Destructor
  ~DummyRenderer(){}  
  //! create texture
  Texture* createTexture(Image* img, Texture::Format fmt){return NULL;}
  //! create rendertarget
  virtual RenderTarget* createRenderTarget(unsigned width, unsigned height) { return NULL; }
  //! set orthographic projection
  virtual void ortho(float left, float right, float bottom, float top, float nearp, float farp){}
  //! reset modelview matrix
  void resetModelView(){}
  //! translate
  void translate(float x, float y, float z){}
  //! scale
  void scale(float x, float y, float z){}
  //! rotate
  virtual void rotate(float angle, float x, float y, float z){}
  //! set render mode
  void renderMode(RendMode rm){}
  //! set blending mode
  void blendFunc(BlendType src, BlendType dest){}
  //! enable blending
  void enableBlend(const bool flag){}
  //! enable backface culling
  void enableBackFaceCulling(const bool flag){}
  //! enable texturing
  void enableTexturing(const bool flag){}
  //! enable lighting
  void enableLighting(const bool flag){}
  //! enable light
  virtual void enableLight(short number, bool flag){}
  //! enable depth test
  virtual void enableDepthTest(const bool flag){}
  //! enable depth write
  virtual void enableDepthWrite(bool flag){}
  //! set color
  void setColor(float r, float g, float b, float a){}
  //! set color
  void setColor(const Color* c){}
  //! set material
  void setMaterial(const Material& mat){}
  //! set light
  virtual void setLight(int number, const Light& lit){}
  //! push matrix
  void pushMatrix(){}
  //! pop matrix
  void popMatrix(){}
  //! multiply matrix
  void multiplyMatrix(const CGE::Matrix& mat){}
  //! get the viewport
  virtual void getViewport(int view[4]){}
  //! get a matrix
  virtual CGE::Matrix getMatrix(MatrixType mt){return CGE::Matrix(CGE::Matrix::Identity);}
  //! swap back and front buffer
  virtual void swapBuffers(){}
  //! switch from view to model matrix
  virtual void switchFromViewToModelTransform(){}
  //! switch matrix stack
  virtual void switchMatrixStack(MatrixType type){}
};

}

#endif

