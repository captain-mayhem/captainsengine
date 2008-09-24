#ifndef DUMMYRENDERER_H
#define DUMMYRENDERER_H

#include "renderer.h"

namespace Graphics{
class DummyRenderer : public Renderer{
public: 
  //! Constructor
  DummyRenderer(){}
  //! Destructor
  ~DummyRenderer(){}  
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
  //! multiply matrix
  void multiplyMatrix(const Math::Matrix& mat){}
  //! get the viewport
  virtual void getViewport(int view[4]){}
  //! get a matrix
  virtual Matrix getMatrix(MatrixType mt){return Matrix(Matrix::Identity);}
  //! swap back and front buffer
  virtual void swapBuffers(){}
};

}

#endif

