#ifndef CGE_TEXTURE_LOADER_H
#define CGE_TEXTURE_LOADER_H

#include <cge_export.h>

#include <memory>
#include <string>

namespace CGE{
namespace Gfx{

class RenderDevice;
class Texture;

//! loads an image file (an asset) as an RGBA8 texture with a full mip chain; nullptr if it can't
/*! With opacity, the texture is white with the image's first channel as
    alpha, as a material's opacity texture needs. */
CGE_API std::unique_ptr<Texture> loadTexture(RenderDevice& device, const std::string& file, bool opacity = false);

}
}

#endif
