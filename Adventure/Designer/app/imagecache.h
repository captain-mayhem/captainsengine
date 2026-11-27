#ifndef SD_IMAGECACHE_H
#define SD_IMAGECACHE_H

#include <map>
#include <memory>
#include <string>

#include <imgui.h>

#include <model/project.h>

namespace CGE{
namespace Gfx{
class Sampler;
class Texture;
}
}

namespace sd{

//! the project's images as textures for ImGui, loaded when first shown
class ImageCache{
public:
  struct Image{
    //! null if the image can't be found or read
    CGE::Gfx::Texture* texture = nullptr;
    int width = 0;
    int height = 0;
    ImTextureID id() const;
  };
  ImageCache();
  ~ImageCache();
  //! the image of that name (the project's images) for a project in folder
  const Image& get(const Project& project, const std::string& folder, const std::string& name);
  //! drops the textures, e.g. for another project or after an image changed
  void clear();
  //! nearest filtering, for magnified pixels
  CGE::Gfx::Sampler* nearestSampler();
private:
  std::map<std::string, Image> images_;
  std::map<std::string, std::unique_ptr<CGE::Gfx::Texture>> textures_;
  std::unique_ptr<CGE::Gfx::Sampler> nearest_;
};

}

#endif
