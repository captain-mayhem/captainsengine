#ifndef DXTEXTURE_H
#define DXTEXTURE_H

#ifdef WIN32

#include <d3d11.h>
#include "../texture.h"

namespace CGE{

class DXTexture : public Texture {
public:
  DXTexture();
  virtual ~DXTexture();
  void activate()const;
  virtual void deactivate()const;
  virtual bool loadFromFile(std::string const& file);
  virtual bool createEmpty(unsigned width, unsigned height, Format fmt);
  static DXGI_FORMAT dxformat(Format fmt);
  ID3D11Texture2D* getHandle() { return mTexture; }
protected:
  ID3D11Texture2D* mTexture;
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
