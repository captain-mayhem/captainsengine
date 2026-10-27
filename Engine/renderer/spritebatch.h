#ifndef CGE_SPRITE_BATCH_H
#define CGE_SPRITE_BATCH_H

#include <cge_export.h>

#include <memory>

#include "drawcontext.h"
#include "material.h"

namespace CGE{

class Matrix;

namespace Gfx{

class Buffer;
class Sampler;
class Shader;
class Texture;
struct AtlasRegion;

//! texture coordinates of a sprite's top left (u0, v0) and bottom right (u1, v1) corner
/*! Images are uploaded with their top row first, so (0, 0) to (1, 1) is
    the whole image the right way round. */
struct TexRect{
  float u0 = 0;
  float v0 = 0;
  float u1 = 1;
  float v1 = 1;
};

//! where and how SpriteBatch::draw places a sprite
/*! The sprite's top left corner is at its (0, 0) and its bottom right
    corner at (width, height), so the image is upright with a projection
    whose y points down, like the virtual screen and the mouse. */
struct Sprite{
  //! where the origin goes
  float x = 0;
  float y = 0;
  //! z, for depth testing
  float depth = 0;
  //! the size before scaling
  float width = 0;
  float height = 0;
  //! the point at (x, y), which the sprite is rotated and scaled around, as fractions of the size
  float originX = 0;
  float originY = 0;
  //! degrees from +x towards +y (clockwise when y points down)
  float rotation = 0;
  float scaleX = 1;
  float scaleY = 1;
  //! multiplied with the texture
  Color color = Color(1, 1, 1, 1);
  //! mirror the image horizontally or vertically (in place, around the origin's axes)
  bool flipX = false;
  bool flipY = false;
};

//! Draws many textured, coloured, rotated and scaled quads in few draw calls
/*! Sprites are collected and drawn together when the texture, the sampler
    or the blend mode changes, when the batch is full, and on flush. Each
    sprite is an instance of the unit quad (BuiltinShader::Sprite) and its
    corners are computed on the CPU, so sprites of one texture are one draw
    call however they are placed.
    The sprites are drawn with the context's projection and view, depth and
    colour write state at the time of the flush, without culling. Call flush
    before changing these and before the pass ends; sprites that are still
    waiting when the next frame begins are dropped with an error.
    The instance data goes into vertex buffers that are reused from one frame
    to the next, so each draw keeps the data it was issued with. */
class CGE_API SpriteBatch{
public:
  //! capacity: the most sprites in one draw call
  explicit SpriteBatch(DrawContext& context, int capacity = 4096);
  //! deletes its buffers and textures; before the device goes
  ~SpriteBatch();
  SpriteBatch(const SpriteBatch&) = delete;
  SpriteBatch& operator=(const SpriteBatch&) = delete;

  DrawContext& getContext() const;

  //! blending of the sprites drawn next; Alpha at the start
  void setBlend(BlendMode mode);
  BlendMode getBlend() const;
  //! the sampler of the sprites drawn next; nullptr for the default one
  /*! The default one filters linearly, between mip levels too, and clamps
      to the edge. */
  void setSampler(Sampler* sampler);
  Sampler* getSampler() const;
  //! draws the sprites drawn next as distance fields, with their edge at edge; negative for normal sprites (the start)
  /*! The texture's alpha is then the distance to a shape's edge, more
      inside, e.g. FreeType's distance field glyphs (Font). Where it is
      edge, the sprite's edge is drawn, smoothed over about a pixel; the
      sprite's colour fills the shape. A smaller edge makes the shape
      bolder, which draws outlines. */
  void setDistanceField(float edge);
  float getDistanceField() const;

  //! the unit quad from (0, 0) to (1, 1) in a 2D transformation
  /*! The quad's corner (0, 0) gets (rect.u0, rect.v0) and its corner (1, 1)
      gets (rect.u1, rect.v1). transform's z-translation is the depth; its
      z-axis is ignored. nullptr as texture draws in the colour alone. */
  void draw(Texture* texture, const Matrix& transform, const TexRect& rect = TexRect(), const Color& color = Color(1, 1, 1, 1));
  //! the rectangle from (x, y) to (x+width, y+height)
  void draw(Texture* texture, float x, float y, float width, float height, const TexRect& rect = TexRect(),
    const Color& color = Color(1, 1, 1, 1));
  void draw(Texture* texture, const Sprite& sprite, const TexRect& rect = TexRect());
  //! a region of a texture atlas; sprite's size is the region's untrimmed size
  /*! Trimmed regions are drawn where they were in the untrimmed image.
      AtlasRegion::sprite gives a sprite of the region's size and pivot. */
  void draw(const AtlasRegion& region, const Sprite& sprite);
  //! a region of a texture atlas at its size, with its pivot at (x, y)
  void draw(const AtlasRegion& region, float x, float y, const Color& color = Color(1, 1, 1, 1));

  //! draws the sprites collected so far; inside a pass
  void flush();
  //! the sprites waiting to be drawn
  int getPending() const;

  //! what the batch drew in the current frame
  struct Stats{
    //! draw calls
    int draws = 0;
    int sprites = 0;
  };
  Stats getStats() const;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}
}

#endif
