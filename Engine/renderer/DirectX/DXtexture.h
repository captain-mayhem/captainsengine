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
  void activate(unsigned stage=0)const;
  virtual void deactivate(unsigned stage=0)const;
  virtual bool createFromImage(Image* img, Format fmt);
  virtual bool createEmpty(unsigned width, unsigned height, Format fmt);
  virtual Texture* copy();
  static DXGI_FORMAT dxformat(Format fmt);
  ID3D11Texture2D* getHandle() const { return mTexture; }
  virtual void update(int x, int y, unsigned width, unsigned height, void* data);
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
