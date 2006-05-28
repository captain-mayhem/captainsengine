#ifndef DXTEXTURE_H
#define DXTEXTURE_H

#include <d3d9.h>
#include "texture.h"

namespace Graphics{

class DXTexture : Texture {
public:
  DXTexture(string filename);
  virtual ~DXTexture(){}
protected:
  virtual bool load(string filename)=0;
  LPDIRECT3DDEVICE9 device_;
  IDirect3DTexture9* tex_;
};

}

#endif