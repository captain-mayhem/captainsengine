#ifndef DXTEXTURE_H
#define DXTEXTURE_H

#ifdef WIN32

#include <d3d9.h>
#include "../texture.h"

namespace CGE{

class DXTexture : public Texture {
public:
  DXTexture(string filename);
  virtual ~DXTexture(){}
  void activate();
  virtual void deactivate();
protected:
  bool load(string filename);
  LPDIRECT3DDEVICE9 device_;
  IDirect3DTexture9* tex_;
};


}


#endif

#ifdef UNIX
#include "texture.h"

namespace Graphics{

class DXTexture : public Texture {
public:
  DXTexture(string filename){};
  virtual ~DXTexture(){}
protected:
};

#endif

#endif
