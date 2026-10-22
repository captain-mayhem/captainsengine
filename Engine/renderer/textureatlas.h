#ifndef CGE_TEXTURE_ATLAS_H
#define CGE_TEXTURE_ATLAS_H

#include <cge_export.h>

#include <memory>
#include <string>
#include <vector>

#include "spritebatch.h"

namespace CGE{
namespace Gfx{

class RenderDevice;
class Texture;

//! an image in a texture atlas
/*! Sizes and positions are in the atlas's units: its pixels divided by its
    scale, so the images of a 2x atlas have the same sizes as those of the
    1x one. */
struct AtlasRegion{
  //! the image's name, without a .png/.jpg/.bmp/.gif extension
  std::string name;
  //! the atlas page it is on
  Texture* texture = nullptr;
  int page = 0;
  //! where it is on the page
  TexRect rect;
  //! the untrimmed image's size
  float width = 0;
  float height = 0;
  //! the part that is in the atlas, inside the untrimmed image from its top left
  /*! The packer cuts transparent borders away; without that, it's the
      whole image. */
  float trimX = 0;
  float trimY = 0;
  float trimWidth = 0;
  float trimHeight = 0;
  //! the point that is placed and rotated around, as fractions of the untrimmed size
  float pivotX = 0.5f;
  float pivotY = 0.5f;

  //! a sprite of the region's size with its pivot at (x, y)
  Sprite sprite(float x, float y) const{
    Sprite s;
    s.x = x;
    s.y = y;
    s.width = width;
    s.height = height;
    s.originX = pivotX;
    s.originY = pivotY;
    return s;
  }
};

//! Images packed into textures, as described by a JSON file
/*! Reads the JSON that the art pipeline's pack_atlas writes and that
    TexturePacker writes as "JSON (Hash)", "JSON (Array)" or "Phaser 3":
    \code
    {"frames": {"jewel_red.png": {
        "frame": {"x": 2, "y": 2, "w": 52, "h": 56},
        "rotated": false, "trimmed": true,
        "spriteSourceSize": {"x": 2, "y": 0, "w": 52, "h": 56},
        "sourceSize": {"w": 56, "h": 56},
        "pivot": {"x": 0.5, "y": 0.5}}},
     "meta": {"image": "jewels.png", "size": {"w": 256, "h": 256}, "scale": "1"}}
    \endcode
    "frames" can also be an array of frames with a "filename" each. Several
    pages are a "textures" array of objects with "image", "scale" and
    "frames" each. Only "frame" is needed; the image is untrimmed without
    "spriteSourceSize" and "sourceSize", and the pivot is the centre without
    "pivot". Rotated frames aren't supported (the packer mustn't rotate).
    The page images are assets relative to the JSON file's folder; they're
    loaded as RGBA8 textures with a full mip chain. */
class CGE_API TextureAtlas{
public:
  explicit TextureAtlas(RenderDevice& device);
  //! deletes the pages; before the device goes
  ~TextureAtlas();
  TextureAtlas(const TextureAtlas&) = delete;
  TextureAtlas& operator=(const TextureAtlas&) = delete;

  //! loads an atlas (an asset) and its pages, replacing what was loaded before
  /*! false (and error messages) if the file or a page can't be read; the
      frames that could be read are kept. */
  bool load(const std::string& file);
  //! reads an atlas from JSON text; the page images are assets relative to dir
  bool loadFromText(const std::string& json, const std::string& dir);
  //! deletes the regions and the pages
  void clear();

  //! the region of an image name; nullptr if there is none
  const AtlasRegion* find(const std::string& name) const;
  //! the regions named prefix followed by a number, in the numbers' order
  /*! For animations: "fairy_fly_" finds fairy_fly_0, fairy_fly_1, ... */
  std::vector<const AtlasRegion*> findSequence(const std::string& prefix) const;
  //! all regions, in the file's order
  const std::vector<AtlasRegion>& getRegions() const;
  int getPageCount() const;
  Texture* getPage(int index) const;
  //! the atlas pixels per unit (e.g. 2 for the 2x art) of the first page; 1 without pages
  /*! Each page can have its own scale; the regions are converted with their page's. */
  float getScale() const;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}
}

#endif
