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
  if(!D3DXCreateTextureFromFile(device_, filename.c_str(), &tex_))
    return false;
  return true;
}