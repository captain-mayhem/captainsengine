#if defined WIN32 && !defined _NODIRECTX

#include "../../system/engine.h"
#include <d3d11.h>
#include "DXrenderer.h"
#include "DXtexture.h"
#include "../../image/loader.h"
#include "../../image/image.h"

using namespace CGE; 

DXTexture::DXTexture(string filename) : Texture(filename) {
  mDevice = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getDevice();
  load(filename);
}

DXTexture::~DXTexture(){
  mTex->Release();
  mState->Release();
}

bool DXTexture::load(string filename){
  Image *img = NULL;

  img = ImageLoader::load(filename.c_str());

  if (!img)
    return false;

  if (img->getNumChannels() == 3){
    img->convertFormat(4);
  }

  D3D11_TEXTURE2D_DESC desc;
  ZeroMemory(&desc, sizeof(desc));
  desc.Width = img->getWidth();
  desc.Height = img->getHeight();
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  //desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  D3D11_SUBRESOURCE_DATA data;
  data.pSysMem = img->getData();
  data.SysMemPitch = img->getRowSpan();
  data.SysMemSlicePitch = img->getImageSize();

  ID3D11Texture2D* tex;
  HRESULT res = mDevice->CreateTexture2D(&desc, &data, &tex);
  if (!SUCCEEDED(res))
    return false;
  mDevice->CreateShaderResourceView(tex, NULL, &mTex);
  tex->Release();

  D3D11_SAMPLER_DESC samp;
  ZeroMemory(&samp, sizeof(samp));
  samp.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  samp.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  samp.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  samp.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  samp.MaxLOD = D3D11_FLOAT32_MAX;
  mDevice->CreateSamplerState(&samp, &mState);

  return true;
}

void DXTexture::activate()const{
  ID3D11DeviceContext* ctx = static_cast< DXRenderer* >(Engine::instance()->getRenderer())->getContext();
  ctx->PSSetShaderResources(0, 1, &mTex);
  ctx->PSSetSamplers(0, 1, &mState);
}

void DXTexture::deactivate()const{
  
}

#endif
