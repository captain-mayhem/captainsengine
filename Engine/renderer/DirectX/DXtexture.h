#ifndef DXTEXTURE_H
#define DXTEXTURE_H

#ifdef WIN32

#include <d3d11.h>
#include "../texture.h"

namespace CGE{

class DXTexture : public Texture {
public:
  DXTexture(string filename);
  virtual ~DXTexture();
  void activate()const;
  virtual void deactivate()const;
protected:
  bool load(string filename);
  ID3D11Device* mDevice;
  ID3D11ShaderResourceView* mTex;
  ID3D11SamplerState* mState;
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
