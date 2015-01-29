#ifndef CGE_RENDERTARGET_H
#define CGE_RENDERTARGET_H

#include <renderer/texture.h>

namespace CGE{

class RenderTarget{
public:
  virtual ~RenderTarget() {}
  virtual void create();
};

}

#endif
