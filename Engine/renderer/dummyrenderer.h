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
    //! set render mode
    void renderMode(RendMode rm){}
    //! set blending mode
    void blendFunc(BlendType src, BlendType dest){}
    //! enable blending
    void enableBlend(const bool flag){}
    //! set color
    void setColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a){}
    //! set color
    void setColor(const Color* c){}
    //! push matrix
    void pushMatrix(){}
    //! pop matrix
    void popMatrix(){}
  };
}

#endif
