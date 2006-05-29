#include "../system/engine.h"
#include <d3dx9.h>
#include "DXrenderer.h"
#include "DXtexture.h"

using namespace Graphics; 

DXTexture::DXTexture(string filename) : Texture(filename) {
  device_ = dynamic_cast< ::Graphics::DXRenderer* >(::System::Engine::instance()->getRenderer())->getDevice();
  load(filename);
}

bool DXTexture::load(string filename){
  //if(!D3DXCreateTextureFromFile(device_, filename.c_str(), &tex_))
  //  return false;
  if (!D3DXCreateTextureFromFileEx(device_, filename.c_str(), D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
    0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, 
    D3DX_FILTER_TRIANGLE | D3DX_FILTER_MIRROR | D3DX_FILTER_DITHER,
    D3DX_FILTER_BOX | D3DX_FILTER_MIRROR | D3DX_FILTER_DITHER,
    0, NULL, NULL, &tex_)){
      return false;
    }
  return true;
}

void DXTexture::activate(){
  device_->SetTexture(0, tex_);
}