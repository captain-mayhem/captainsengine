#include "textureloader.h"

#include <vector>

#include "renderdevice.h"
#include "../image/image.h"
#include "../image/loader.h"
#include "../io/Tracing.h"

TR_CHANNEL(CGE_TextureLoader);

namespace CGE{
namespace Gfx{

std::unique_ptr<Texture> loadTexture(RenderDevice& device, const std::string& file, bool opacity){
  TR_USE(CGE_TextureLoader);
  std::unique_ptr<Image> image(ImageLoader::load(file.c_str()));
  if (!image){
    TR_ERROR("cannot load texture %s", file.c_str());
    return nullptr;
  }
  TextureDesc desc;
  desc.width = (int)image->getWidth();
  desc.height = (int)image->getHeight();
  desc.mipLevels = 0;
  std::vector<unsigned char> rgba;
  if (opacity){
    unsigned channels = image->getNumChannels();
    unsigned pixels = image->getWidth()*image->getHeight();
    rgba.resize(pixels*4);
    for (unsigned i = 0; i < pixels; ++i){
      rgba[4*i] = rgba[4*i+1] = rgba[4*i+2] = 255;
      rgba[4*i+3] = image->getData()[channels*i];
    }
  }
  else{
    image->convertFormat(4);
    rgba.assign(image->getData(), image->getData()+image->getWidth()*image->getHeight()*4);
  }
  std::unique_ptr<Texture> texture = device.createTexture(desc, rgba.data());
  if (texture)
    device.generateMipmaps(texture.get());
  return texture;
}

}
}
