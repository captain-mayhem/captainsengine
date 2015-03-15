#ifndef CGE_RENDERTARGET_H
#define CGE_RENDERTARGET_H

#include <renderer/texture.h>

namespace CGE{

class RenderTarget{
  friend class GL2Renderer;
public:
  RenderTarget() : mOldTarget(NULL) {}
  virtual ~RenderTarget() {}
  void activate() { mOldTarget = mCurrTarget; bind(); mCurrTarget = this; }
  void deactivate() { if (mOldTarget) mOldTarget->bind(); mCurrTarget = mOldTarget; }
  static RenderTarget* getCurrent() { return mCurrTarget; }

  virtual void addTexture(Texture::Format fmt) = 0;
  virtual void addTexture(const CGE::Texture& tex) = 0;
  virtual void addRenderbuffer(Texture::Format fmt) = 0;
  virtual bool isComplete() = 0;
  virtual Texture* getTexture(unsigned idx) = 0;
  virtual unsigned getNumTextures() = 0;
  void drawFullscreen(bool transform);
protected:
  virtual void bind() = 0;

  static RenderTarget* mCurrTarget;
  RenderTarget* mOldTarget;
};

}

#endif
