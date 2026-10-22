#include "textureatlas.h"

#include <algorithm>
#include <cstdlib>
#include <unordered_map>

#include "renderdevice.h"
#include "textureloader.h"
#include "../io/json.h"
#include "../io/Tracing.h"
#include "../platform/assets.h"

using namespace CGE;
using namespace CGE::Gfx;

TR_CHANNEL(CGE_TextureAtlas);

namespace{

//! the name without an image file extension
std::string regionName(const std::string& file){
  static const char* extensions[] = {".png", ".jpg", ".jpeg", ".bmp", ".gif"};
  size_t dot = file.rfind('.');
  if (dot == std::string::npos)
    return file;
  std::string ext = file.substr(dot);
  std::transform(ext.begin(), ext.end(), ext.begin(), [](char c){return (char)tolower((unsigned char)c);});
  for (const char* known : extensions){
    if (ext == known)
      return file.substr(0, dot);
  }
  return file;
}

//! a number that TexturePacker may also write as a string ("scale": "0.5")
float numberOf(const JsonValue& value, float def){
  if (value.isString() && !value.getString().empty())
    return (float)std::atof(value.getString().c_str());
  return value.getFloat(def);
}

}

struct TextureAtlas::Impl{
  explicit Impl(RenderDevice& dev) : device(dev){}

  //! reads the frames of one page; false if one couldn't be read
  bool readFrames(const JsonValue& frames, int page, float scale){
    TR_USE(CGE_TextureAtlas);
    bool ok = true;
    auto readFrame = [&](const std::string& file, const JsonValue& frame){
      const JsonValue& rect = frame["frame"];
      if (!rect.isObject()){
        TR_ERROR("frame %s has no \"frame\" rectangle", file.c_str());
        ok = false;
        return;
      }
      if (frame["rotated"].getBool()){
        TR_ERROR("frame %s is rotated, which isn't supported", file.c_str());
        ok = false;
        return;
      }
      Texture* tex = pages[page].get();
      float pageWidth = (float)tex->getDesc().width;
      float pageHeight = (float)tex->getDesc().height;
      float x = rect["x"].getFloat();
      float y = rect["y"].getFloat();
      float w = rect["w"].getFloat();
      float h = rect["h"].getFloat();
      AtlasRegion region;
      region.name = regionName(file);
      region.texture = tex;
      region.page = page;
      region.rect.u0 = x/pageWidth;
      region.rect.v0 = y/pageHeight;
      region.rect.u1 = (x+w)/pageWidth;
      region.rect.v1 = (y+h)/pageHeight;
      const JsonValue& trim = frame["spriteSourceSize"];
      const JsonValue& source = frame["sourceSize"];
      region.trimX = trim["x"].getFloat(0)/scale;
      region.trimY = trim["y"].getFloat(0)/scale;
      region.trimWidth = w/scale;
      region.trimHeight = h/scale;
      region.width = source["w"].getFloat(w)/scale;
      region.height = source["h"].getFloat(h)/scale;
      const JsonValue& pivot = frame["pivot"];
      region.pivotX = pivot["x"].getFloat(0.5f);
      region.pivotY = pivot["y"].getFloat(0.5f);
      if (names.count(region.name))
        TR_WARN("frame %s is in the atlas twice; the first one is used", region.name.c_str());
      else
        names[region.name] = regions.size();
      regions.push_back(region);
    };
    if (frames.isObject()){
      for (const auto& member : frames.getMembers())
        readFrame(member.first, member.second);
    }
    else if (frames.isArray()){
      for (const JsonValue& frame : frames.getElements())
        readFrame(frame["filename"].getString(), frame);
    }
    else{
      TR_ERROR("the atlas has no frames");
      ok = false;
    }
    return ok;
  }

  //! loads a page image and reads its frames
  bool readPage(const JsonValue& image, const JsonValue& frames, const JsonValue& scaleValue, const std::string& dir){
    TR_USE(CGE_TextureAtlas);
    if (!image.isString() || image.getString().empty()){
      TR_ERROR("an atlas page has no image");
      return false;
    }
    std::string file = image.getString();
    if (!dir.empty() && !Assets::isAbsolute(file))
      file = dir + "/" + file;
    std::unique_ptr<Texture> texture = loadTexture(device, file);
    if (!texture)
      return false;
    float scale = numberOf(scaleValue, 1);
    if (scale <= 0)
      scale = 1;
    if (pages.empty())
      firstScale = scale;
    pages.push_back(std::move(texture));
    return readFrames(frames, (int)pages.size()-1, scale);
  }

  RenderDevice& device;
  std::vector<std::unique_ptr<Texture>> pages;
  std::vector<AtlasRegion> regions;
  std::unordered_map<std::string, size_t> names;
  float firstScale = 1;
};

TextureAtlas::TextureAtlas(RenderDevice& device) : impl_(std::make_unique<Impl>(device)){
}

TextureAtlas::~TextureAtlas(){
}

bool TextureAtlas::load(const std::string& file){
  TR_USE(CGE_TextureAtlas);
  std::string text;
  if (!Assets::loadText(file, text)){
    clear();
    TR_ERROR("cannot read the texture atlas %s", file.c_str());
    return false;
  }
  size_t slash = file.find_last_of("/\\");
  std::string dir = slash == std::string::npos ? std::string() : file.substr(0, slash);
  if (!loadFromText(text, dir)){
    TR_ERROR("the texture atlas %s couldn't be read completely", file.c_str());
    return false;
  }
  return true;
}

bool TextureAtlas::loadFromText(const std::string& json, const std::string& dir){
  TR_USE(CGE_TextureAtlas);
  clear();
  JsonValue root;
  std::string error;
  if (!JsonValue::parse(json, root, &error)){
    TR_ERROR("the texture atlas isn't valid JSON: %s", error.c_str());
    return false;
  }
  const JsonValue& textures = root["textures"];
  if (textures.isArray()){
    //several pages (TexturePacker's Phaser 3 format)
    bool ok = textures.size() > 0;
    for (const JsonValue& page : textures.getElements())
      ok = impl_->readPage(page["image"], page["frames"], page["scale"], dir) && ok;
    return ok;
  }
  const JsonValue& meta = root["meta"];
  return impl_->readPage(meta["image"], root["frames"], meta["scale"], dir);
}

void TextureAtlas::clear(){
  impl_->regions.clear();
  impl_->names.clear();
  impl_->pages.clear();
  impl_->firstScale = 1;
}

const AtlasRegion* TextureAtlas::find(const std::string& name) const{
  auto iter = impl_->names.find(name);
  if (iter == impl_->names.end())
    return nullptr;
  return &impl_->regions[iter->second];
}

std::vector<const AtlasRegion*> TextureAtlas::findSequence(const std::string& prefix) const{
  std::vector<std::pair<long, const AtlasRegion*>> numbered;
  for (const AtlasRegion& region : impl_->regions){
    const std::string& name = region.name;
    if (name.size() <= prefix.size() || name.compare(0, prefix.size(), prefix) != 0)
      continue;
    std::string rest = name.substr(prefix.size());
    if (rest.find_first_not_of("0123456789") != std::string::npos)
      continue;
    //a duplicate name is found once, like find does
    if (find(name) != &region)
      continue;
    numbered.emplace_back(std::atol(rest.c_str()), &region);
  }
  std::stable_sort(numbered.begin(), numbered.end(), [](const auto& a, const auto& b){return a.first < b.first;});
  std::vector<const AtlasRegion*> result;
  for (const auto& entry : numbered)
    result.push_back(entry.second);
  return result;
}

const std::vector<AtlasRegion>& TextureAtlas::getRegions() const{
  return impl_->regions;
}

int TextureAtlas::getPageCount() const{
  return (int)impl_->pages.size();
}

Texture* TextureAtlas::getPage(int index) const{
  if (index < 0 || index >= (int)impl_->pages.size())
    return nullptr;
  return impl_->pages[index].get();
}

float TextureAtlas::getScale() const{
  return impl_->firstScale;
}
