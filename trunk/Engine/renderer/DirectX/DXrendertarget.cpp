#include "DXrendertarget.h"
#include "../../system/engine.h"
#include "../../window/window.h"
#include "DXrenderer.h"
#include "DXtexture.h"

using namespace CGE;

DXRenderTarget::DXRenderTarget(IDXGISwapChain* chain) : mDS(NULL){
  mCtx = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getContext();
  ID3D11Device* dev = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getDevice();

  AppWindow* wnd = Engine::instance()->getWindow();
  mWidth = wnd->getWidth();
  mHeight = wnd->getHeight();

  ID3D11Texture2D* backbuffer;
  chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backbuffer);
  ID3D11RenderTargetView* rt;
  dev->CreateRenderTargetView(backbuffer, NULL, &rt);
  mRT.push_back(rt);
  backbuffer->Release();
}

DXRenderTarget::DXRenderTarget(int width, int height) : mWidth(width), mHeight(height), mDS(NULL){
  mCtx = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getContext();
}

DXRenderTarget::~DXRenderTarget(){
  for (unsigned i = 0; i < mRT.size(); ++i){
    SAFE_RELEASE(mRT[i]);
  }
  for (unsigned i = 0; i < mRTTextures.size(); ++i){
    delete mRTTextures[i];
  }
  SAFE_RELEASE(mDS);
}

void DXRenderTarget::addTexture(Texture::Format fmt){
  ID3D11Device* dev = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getDevice();

  DXTexture* tex = new DXTexture();
  tex->createEmpty(mWidth, mHeight, fmt);
  mRTTextures.push_back(tex);

  addTexture(*tex);
}

void DXRenderTarget::addTexture(const CGE::Texture& tex){
  ID3D11Device* dev = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getDevice();
  if (tex.getFormat() != Texture::DEPTH){
    D3D11_RENDER_TARGET_VIEW_DESC rtvd;
    ZeroMemory(&rtvd, sizeof(rtvd));
    rtvd.Format = DXTexture::dxformat(tex.getFormat());
    rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvd.Texture2D.MipSlice = 0;
    ID3D11RenderTargetView* rtv;
    dev->CreateRenderTargetView(((const DXTexture&)tex).getHandle(), &rtvd, &rtv);
    mRT.push_back(rtv);
  }
  else{
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
    ZeroMemory(&dsvd, sizeof(dsvd));
    dsvd.Format = DXTexture::dxformat(tex.getFormat());
    dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvd.Texture2D.MipSlice = 0;
    dev->CreateDepthStencilView(((const DXTexture&)tex).getHandle(), &dsvd, &mDS);
  }
}

void DXRenderTarget::addRenderbuffer(Texture::Format fmt){
  ID3D11Device* dev = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getDevice();

  if (fmt == Texture::DEPTH){
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = mWidth;
    desc.Height = mHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXTexture::dxformat(fmt);
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = 0;
    if (fmt == Texture::DEPTH)
      desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
    else
      desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    ID3D11Texture2D* tex;
    HRESULT res = dev->CreateTexture2D(&desc, NULL, &tex);
    if (!SUCCEEDED(res))
      return;

    if (fmt == Texture::DEPTH){
      D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
      ZeroMemory(&dsvd, sizeof(dsvd));
      dsvd.Format = DXTexture::dxformat(fmt);
      dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
      dsvd.Texture2D.MipSlice = 0;
      dev->CreateDepthStencilView(tex, &dsvd, &mDS);
    }
    tex->Release();
  }
}

bool DXRenderTarget::isComplete(){
  return true;
}

Texture* DXRenderTarget::getTexture(unsigned idx){
  return mRTTextures[idx];
}

void DXRenderTarget::bind(){
  mCtx->OMSetRenderTargets((UINT)mRT.size(), mRT.data(), mDS);
}
