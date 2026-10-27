#ifndef CGE_FONT_H
#define CGE_FONT_H

#include <cge_export.h>

#include <system/types.h>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "material.h"
#include "../math/vector.h"

namespace CGE{

namespace Gfx{
class RenderDevice;
class SpriteBatch;
}

//! where the lines of a text go relative to the x of Font::draw
enum class TextAlign{
  //! they start at x
  Left,
  //! their centres are at x
  Center,
  //! they end at x
  Right,
};

//! how Font::draw draws a text; sizes and offsets are in the projection's units
struct TextStyle{
  Color color = Color(1, 1, 1, 1);
  //! the size (the em square), 0 for the font's size
  float size = 0;
  //! the width of an outline around the glyphs, 0 for none
  float outline = 0;
  Color outlineColor = Color(0, 0, 0, 1);
  //! where the shadow is relative to the text (y down); no shadow if both are 0
  float shadowX = 0;
  float shadowY = 0;
  Color shadowColor = Color(0, 0, 0, 0.6f);
  TextAlign align = TextAlign::Left;
  //! lines wider than this are wrapped at spaces; 0 doesn't wrap
  float wrapWidth = 0;
};

//! how a font is loaded
struct FontDesc{
  //! the size (the em square) in the projection's units, e.g. the virtual screen's
  float size = 16;
  //! the face in a font collection (.ttc)
  int faceIndex = 0;
  //! glyphs as distance fields, rendered once for all sizes; for large text
  /*! They stay sharp at any size and scale, and outlines cost nothing extra,
      but small text looks a little softer than with normal glyphs, and
      outlines are at most about a sixth of the size wide. */
  bool distanceField = false;
  //! synthetic bold and slant, for fonts without a bold or italic face
  bool embolden = false;
  bool oblique = false;
};

//! A TrueType or OpenType font, drawn with FreeType's glyphs through a SpriteBatch
/*! Glyphs are rendered when they are first drawn into the font's atlas
    textures (1024x1024, as many as needed; when a new frame begins with
    four or more, they are emptied and fill again). Normal glyphs are
    rendered at the size they cover on the screen: draw takes the pixels
    per unit from the batch's projection and the pass's viewport, so text
    is sharp at any window size and content scale without the application
    doing anything; this is the only place where a font meets the screen's
    scale. A new size renders new glyphs, so text that changes its size
    every frame should use a distance field font.
    Texts are UTF-8 (bytes that aren't UTF-8 are taken as ISO 8859-1),
    with '\n' between lines. Glyphs come from the font's character map;
    characters it hasn't get its missing glyph (usually a box). Pairs of
    glyphs are kerned with the font's 'kern' table (not with OpenType's
    GPOS, which would need a shaping library).
    A font has textures of its device, so it goes before the device does,
    and after a lost device (IApplication::onDeviceLost) it is loaded anew. */
class CGE_API Font{
public:
  //! a font file of the assets (Assets::load), or the engine's font for an empty path
  /*! The engine's font (getDefaultPath) falls back on the system's
      sans-serif font where the file is missing. nullptr if neither loads. */
  static std::unique_ptr<Font> load(Gfx::RenderDevice& device, const std::string& path = "", const FontDesc& desc = FontDesc());
  //! a font file's contents
  static std::unique_ptr<Font> load(Gfx::RenderDevice& device, std::vector<uint8> data, const FontDesc& desc = FontDesc());
  //! a font of the system (SystemFonts::find, which can substitute another family)
  static std::unique_ptr<Font> loadSystem(Gfx::RenderDevice& device, const std::string& family, const FontDesc& desc = FontDesc(),
    int weight = 400, bool italic = false);
  //! the engine's font as an asset path: fonts/DejaVuSans.ttf
  static const char* getDefaultPath();
  //! the engine's font file (or the system's sans-serif font), e.g. for ImGui; false if there is none
  static bool loadDefaultData(std::vector<uint8>& data);

  ~Font();
  Font(const Font&) = delete;
  Font& operator=(const Font&) = delete;

  //! the family's name, e.g. "DejaVu Sans"
  const std::string& getFamily() const;
  const FontDesc& getDesc() const;
  //! from the baseline up to the top of the tallest glyphs, at a size (0: the font's)
  float getAscent(float size = 0) const;
  //! from the baseline down to the bottom of the lowest glyphs (positive)
  float getDescent(float size = 0) const;
  //! from one line's baseline to the next's
  float getLineHeight(float size = 0) const;
  //! has the font a glyph for the character
  bool hasGlyph(char32_t ch) const;

  //! the size of the text's box as draw lays it out, without the outline and the shadow
  /*! The width of its widest line and the height of its lines. Normal
      glyphs are placed on whole pixels, so the width can differ a little
      from the one drawn on a screen of another scale; this measures at one
      pixel per unit. */
  Vec2f measure(std::string_view text, const TextStyle& style = TextStyle()) const;
  //! draws a text into the batch, with the top of its first line at y (y down)
  /*! x is where each line starts, has its centre or ends (style.align).
      The batch's projection must have y down; it draws with the batch's
      blend mode, and its sampler and distance field setting are kept. */
  void draw(Gfx::SpriteBatch& batch, float x, float y, std::string_view text, const TextStyle& style = TextStyle());

  //! what the font's atlas holds
  struct Stats{
    //! glyphs rendered into it
    int glyphs = 0;
    //! its textures
    int pages = 0;
  };
  Stats getStats() const;

private:
  Font();
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

//! decodes the UTF-8 character at pos and moves pos behind it
/*! Bytes that don't start a valid UTF-8 sequence are taken as ISO 8859-1
    characters, so old 8-bit texts still show. */
CGE_API char32_t nextCharacter(std::string_view text, size_t& pos);

}

#endif
