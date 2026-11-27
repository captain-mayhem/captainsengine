#ifndef SD_BITMAPFONT_H
#define SD_BITMAPFONT_H

#include <cstdint>
#include <string>
#include <vector>

#include "project.h"

// The game's bitmap fonts (fonts.dat), rendered from the system's fonts with
// FreeType as the C# StoryDesigner renders them with GDI+ (its
// Fonts.createBitmapFont and Utilities.drawText): the characters 0x20 to
// 0xFF of Windows-1252 in a grid on 256x256 textures, with the font's
// outline, shadow and fill.

namespace sd{

//! an image of 8-bit channels, rows from the top
struct Bitmap{
  int width = 0;
  int height = 0;
  int channels = 3;
  std::vector<uint8_t> pixels;
  uint8_t* at(int x, int y) {return &pixels[((size_t)y*width+x)*channels];}
  const uint8_t* at(int x, int y) const {return &pixels[((size_t)y*width+x)*channels];}
};

//! a font of the game as fonts.dat holds it
struct BitmapFont{
  //! the textures: the characters in white (fill), grey or black (outline, shadow) on black
  std::vector<Bitmap> images;
  //! the textures' alpha: the same characters in white on black
  std::vector<Bitmap> alphaImages;
  //! a character's cell
  Vec2i cellSize;
  //! the cells per row and the rows per texture
  Vec2i cells;
  //! the 224 characters' advance widths
  std::vector<int> widths;
};

//! the first character and the number of characters in a bitmap font
inline const int kFirstFontChar = 0x20;
inline const int kFontChars = 224;

//! renders a font of the project; false (and why) if its family can't be loaded
bool renderBitmapFont(const FontInfo& info, BitmapFont& font, std::string* error = nullptr);

//! a text (Windows-1252) in a font as the Fonts dialog previews it: its
//! outline, shadow and fill, as RGBA on a transparent image of the given size,
//! each line centred
bool renderFontPreview(const FontInfo& info, const std::vector<std::string>& lines, int width, int height, Bitmap& image, std::string* error = nullptr);

//! a 24-bit BMP file of an image
std::string writeBmp(const Bitmap& image);
//! fontdata.00N: the textures' count, the cell's size, the grid and the widths
std::string fontDataText(const BitmapFont& font);
//! font.00N of fonts.dat: a zip of fontN.bm<i>, fontN.al<i> and fontdata.00N
std::string fontArchive(const BitmapFont& font, int number);

}

#endif
