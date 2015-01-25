#ifndef DXRENDERER_H
#define DXRENDERER_H

#ifndef DIRECTX
#define Dummy DX
#include "../dummyrenderer.h"
#undef Dummy
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <stack>
#include "../../math/vector.h"
#include "../renderer.h"

using std::stack;

namespace CGE{

class VertexBuffer;
class DXShader;

//! The DirectX version of the rendering interface
class DXRenderer : public Renderer{
public: 
  //! Constructor
  DXRenderer();
  //! Destructor
  ~DXRenderer();
  //! get device
  inline ID3D11Device* getDevice() const {return mDevice;}
  //! get context
  inline ID3D11DeviceContext* getContext() const { return mD3d; }
  //! init rendering context
  void initContext(AppWindow* win);
  //! kill rendering context
  void killContext();
  //! start scene
  void initRendering();
  //! render scene
  void renderScene();
  //! resize event
  void resizeScene(int width, int height);
  //! set clear color
  void setClearColor(::CGE::Vector3D color);
  //! clear scene
  void clear(long flags);
  //! get a vertex buffer
  VertexBuffer* createVertexBuffer();
  //! get a index buffer
  virtual IndexBuffer* createIndexBuffer(IndexBuffer::Type t, uint32 size);
  //! get a texture
  Texture* createTexture(string filename);
  //! set lookAt
  void lookAt(const CGE::Vec3f& position, const CGE::Vec3f& look, const CGE::Vec3f& up);
  //! set perspective projection
  void projection(float angle, float aspect, float nearplane, float farplane);
  //! set orthographic projection
  void ortho(float left, float right, float bottom, float top, float nearp, float farp);
  //! reset modelview matrix
  void resetModelView();
  //! translate
  void translate(float x, float y, float z);
  //! scale
  void scale(float x, float y, float z);
  //! rotate
  virtual void rotate(float angle, float x, float y, float z);
  //! set render mode
  void renderMode(RendMode rm);
  //! set blending mode
  void blendFunc(BlendType src, BlendType dest);
  //! enable blending
  void enableBlend(const bool flag);
  //! enable culling
  void enableBackFaceCulling(const bool flag);
  //! enable texturing
  void enableTexturing(const bool flag);
  //! enable lighting
  void enableLighting(const bool flag);
  //! enable light
  virtual void enableLight(short number, bool flag);
  //! enable depth test
  virtual void enableDepthTest(const bool flag);
  //! set color
  void setColor(float r, float g, float b, float a);
  //! set color
  void setColor(const Color* c);
  //! set material
  void setMaterial(const Material& mat);
  //! set light
  virtual void setLight(int number, const Light& lit);
  //! push matrix
  void pushMatrix();
  //! pop matrix
  void popMatrix();
  //! multiply matrix
  void multiplyMatrix(const CGE::Matrix& mat);
  //! get the viewport
  virtual void getViewport(int view[4]);
  //! get a matrix
  virtual CGE::Matrix getMatrix(MatrixType mt);
  //! swap back and front buffer
  virtual void swapBuffers();
  //! switch matrix type
  virtual void switchMatrixStack(MatrixType type);
protected:
  //! the directX context
  ID3D11DeviceContext* mD3d;
  //! the graphics device
  ID3D11Device* mDevice;
  //! the swap chain
  IDXGISwapChain* mSwapchain;
  //! the back buffer
  ID3D11RenderTargetView* mBackBuffer;
  //! the depth buffer
  ID3D11DepthStencilView* mDepthBuffer;
  //! the clear color
  FLOAT mClearColor[4];
  //! the matrix stack
  std::stack<Matrix> mMatrixStack[3];
  //! the matrices
  Matrix mMatrix[3];
  //! the transform type
  MatrixType mMatrixMode;
  //! the default shader
  DXShader* mShader;

  struct PSUniforms{
    Color mModColor;
    int mTextureEnabled;
  };
  //! the engine uniforms
  PSUniforms mPSUniforms;
  //! the blend state
  ID3D11BlendState* mBlendState;
  //! the blend description
  D3D11_BLEND_DESC mBlendDesc;
  //! state of the depth test
  ID3D11DepthStencilState* mDepthState;
};

}

#endif

#endif
