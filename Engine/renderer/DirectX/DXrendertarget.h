#ifndef DX_RENDERTARGET_H
#define DX_RENDERTARGET_H

#include "../rendertarget.h"
#include <vector>
#include <d3d11.h>

namespace CGE{

class DXTexture;

class DXRenderTarget : public RenderTarget{
  friend class DXRenderer;
public:
  DXRenderTarget(IDXGISwapChain* chain);
  DXRenderTarget(int width, int height);
  ~DXRenderTarget();
  virtual void addTexture(Texture::Format fmt);
  virtual void addRenderbuffer(Texture::Format fmt);
  virtual bool isComplete();
  virtual Texture* getTexture(unsigned idx);
  virtual unsigned getNumTextures() { return mRTTextures.size(); }
  ID3D11RenderTargetView* getRenderTargets() { return mRT[0]; }
  ID3D11DepthStencilView* getDepthStencil() { return mDS; }
protected:
  virtual void bind();
private:
  int mWidth;
  int mHeight;
  ID3D11DeviceContext* mCtx;
  std::vector<DXTexture*> mRTTextures;
  std::vector<ID3D11RenderTargetView*> mRT;
  ID3D11DepthStencilView* mDS;
};

}

#endif