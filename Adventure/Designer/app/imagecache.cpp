#include "imagecache.h"

#include <renderer/renderdevice.h>
#include <system/engine.h>
#include <ui/imgui_impl_cge_render.h>

#include <model/images.h>

namespace sd{

ImTextureID ImageCache::Image::id() const{
  return ImGui_ImplCGE_TextureID(texture);
}

ImageCache::ImageCache(){
}

ImageCache::~ImageCache(){
}

const ImageCache::Image& ImageCache::get(const Project& project, const std::string& folder, const std::string& name){
  std::string key = toLower(name);
  auto iter = images_.find(key);
  if (iter != images_.end())
    return iter->second;
  Image& entry = images_[key];
  CGE::Engine* engine = CGE::Engine::instance();
  CGE::Gfx::RenderDevice* device = engine ? engine->getRenderDevice() : nullptr;
  std::unique_ptr<CGE::Image> img = loadProjectImage(project, folder, name);
  if (!img || !device)
    return entry;
  CGE::Gfx::TextureDesc desc;
  desc.width = (int)img->getWidth();
  desc.height = (int)img->getHeight();
  std::unique_ptr<CGE::Gfx::Texture> tex = device->createTexture(desc, img->getData());
  if (!tex)
    return entry;
  entry.texture = tex.get();
  entry.width = desc.width;
  entry.height = desc.height;
  textures_[key] = std::move(tex);
  return entry;
}

void ImageCache::clear(){
  images_.clear();
  textures_.clear();
}

CGE::Gfx::Sampler* ImageCache::nearestSampler(){
  if (!nearest_){
    CGE::Engine* engine = CGE::Engine::instance();
    if (engine && engine->getRenderDevice()){
      CGE::Gfx::SamplerDesc desc;
      desc.minFilter = CGE::Gfx::Filter::Nearest;
      desc.magFilter = CGE::Gfx::Filter::Nearest;
      desc.addressU = desc.addressV = desc.addressW = CGE::Gfx::AddressMode::ClampToEdge;
      nearest_ = engine->getRenderDevice()->createSampler(desc);
    }
  }
  return nearest_.get();
}

}
