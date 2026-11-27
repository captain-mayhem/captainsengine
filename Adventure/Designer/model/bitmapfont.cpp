#include "bitmapfont.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <memory>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <io/ZipWriter.h>
#include <platform/systemfonts.h>

namespace sd{

namespace{

//! a Windows-1252 character's code point
uint32_t codePoint(unsigned char ch){
  std::string utf8 = toUtf8(std::string(1, (char)ch));
  const unsigned char* s = (const unsigned char*)utf8.data();
  if (utf8.size() == 1)
    return s[0];
  if (utf8.size() == 2)
    return ((s[0] & 0x1F) << 6) | (s[1] & 0x3F);
  return ((s[0] & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
}

//! a glyph's coverage, placed relative to the pen on the baseline
struct Glyph{
  int left = 0;
  int top = 0;
  int width = 0;
  int height = 0;
  int advance = 0;
  std::vector<uint8_t> coverage;
};

//! a float RGBA canvas, painted over as GDI+ paints
struct Canvas{
  int width;
  int height;
  std::vector<float> rgba;
  Canvas(int w, int h, float r, float g, float b, float a) : width(w), height(h), rgba((size_t)w*h*4){
    for (size_t i = 0; i < rgba.size(); i += 4){
      rgba[i] = r;
      rgba[i+1] = g;
      rgba[i+2] = b;
      rgba[i+3] = a;
    }
  }
};

//! the colour of a fill at a pixel, from its row
struct Brush{
  enum Kind{Solid, Gradient, Hatch};
  Kind kind = Solid;
  float color[3] = {1, 1, 1};
  float alpha = 1;
  //! the gradient's first and last row and colours
  float y0 = 0;
  float y1 = 1;
  float from = 1;
  float to = 0.5f;
  void at(int y, float* rgb) const{
    switch (kind){
      case Solid:
        rgb[0] = color[0];
        rgb[1] = color[1];
        rgb[2] = color[2];
        return;
      case Gradient:{
        float t = y1 > y0 ? ((float)y+0.5f-y0)/(y1-y0) : 0;
        t = std::clamp(t, 0.0f, 1.0f);
        float v = from + (to-from)*t;
        rgb[0] = rgb[1] = rgb[2] = v;
        return;
      }
      case Hatch:{
        //HatchStyle.NarrowHorizontal: light grey lines on every other row, on white
        float v = (y & 1) == 0 ? 211.0f/255 : 1.0f;
        rgb[0] = rgb[1] = rgb[2] = v;
        return;
      }
    }
  }
};

class FontRenderer{
public:
  ~FontRenderer(){
    if (face_)
      FT_Done_Face(face_);
    if (lib_)
      FT_Done_FreeType(lib_);
  }
  bool load(const FontInfo& info, std::string* error){
    info_ = info;
    CGE::SystemFonts::FontData data;
    if (!CGE::SystemFonts::find(toUtf8(info.name), data, info.bold ? 700 : 400, info.italic) || data.data.empty()){
      if (error)
        *error = "The font " + toUtf8(info.name) + " isn't installed.";
      return false;
    }
    data_ = std::move(data.data);
    if (FT_Init_FreeType(&lib_) != 0 || FT_New_Memory_Face(lib_, data_.data(), (FT_Long)data_.size(), data.faceIndex, &face_) != 0){
      if (error)
        *error = "The font " + toUtf8(info.name) + " can't be read.";
      return false;
    }
    //GDI+'s points at 96 dots per inch
    int size = info.size > 0 ? info.size : 12;
    FT_Set_Char_Size(face_, 0, size*64, 96, 96);
    ascent_ = (int)std::ceil(face_->size->metrics.ascender/64.0);
    lineHeight_ = (int)std::ceil(face_->size->metrics.height/64.0);
    //GDI+'s DrawString leaves a sixth of the em before the text
    padding_ = (int)std::lround(size*96.0/72.0/6.0);
    outline_ = info.outline == 1 || info.outline == 2 ? 1 : info.outline == 3 ? 2 : 0;
    shadow_ = info.shadow != 0 ? 3 : 0;
    return true;
  }
  const Glyph& glyph(unsigned char ch){
    auto it = glyphs_.find(ch);
    if (it != glyphs_.end())
      return it->second;
    Glyph& g = glyphs_[ch];
    FT_UInt index = FT_Get_Char_Index(face_, codePoint(ch));
    if (FT_Load_Glyph(face_, index, FT_LOAD_DEFAULT) == 0 && FT_Render_Glyph(face_->glyph, FT_RENDER_MODE_NORMAL) == 0){
      FT_GlyphSlot slot = face_->glyph;
      g.advance = (int)std::lround(slot->advance.x/64.0);
      g.left = slot->bitmap_left;
      g.top = slot->bitmap_top;
      g.width = (int)slot->bitmap.width;
      g.height = (int)slot->bitmap.rows;
      g.coverage.resize((size_t)g.width*g.height);
      for (int y = 0; y < g.height; ++y){
        for (int x = 0; x < g.width; ++x)
          g.coverage[(size_t)y*g.width+x] = slot->bitmap.buffer[y*slot->bitmap.pitch+x];
      }
    }
    return g;
  }
  //! paints a glyph with its top left (of the line) at x, y
  void paint(Canvas& canvas, int x, int y, unsigned char ch, const Brush& brush){
    const Glyph& g = glyph(ch);
    int ox = x + padding_ + g.left;
    int oy = y + ascent_ - g.top;
    for (int gy = 0; gy < g.height; ++gy){
      int py = oy+gy;
      if (py < 0 || py >= canvas.height)
        continue;
      float rgb[3];
      brush.at(py, rgb);
      for (int gx = 0; gx < g.width; ++gx){
        int px = ox+gx;
        if (px < 0 || px >= canvas.width)
          continue;
        float a = g.coverage[(size_t)gy*g.width+gx]/255.0f*brush.alpha;
        if (a <= 0)
          continue;
        float* dst = &canvas.rgba[((size_t)py*canvas.width+px)*4];
        for (int c = 0; c < 3; ++c)
          dst[c] = dst[c]*(1-a) + rgb[c]*a;
        dst[3] = dst[3] + a*(1-dst[3]);
      }
    }
  }
  //! Utilities.drawText: the outline, the shadow and the fill of a text (allWhite: all in white, for the alpha)
  void drawText(Canvas& canvas, int x, int y, const std::string& text, bool allWhite){
    Brush dark;
    float shade = allWhite ? 1.0f : 0.0f;
    dark.color[0] = dark.color[1] = dark.color[2] = shade;
    auto layer = [&](int dx, int dy, const Brush& brush){
      int pen = x+dx;
      for (char c : text){
        paint(canvas, pen, y+dy, (unsigned char)c, brush);
        pen += glyph((unsigned char)c).advance;
      }
    };
    if (info_.outline == 1 || info_.outline == 3){
      layer(-1, -1, dark);
      layer(0, -1, dark);
      layer(-1, 0, dark);
    }
    if (info_.outline == 2 || info_.outline == 3){
      layer(1, 1, dark);
      layer(0, 1, dark);
      layer(1, 0, dark);
    }
    if (info_.shadow != 0){
      Brush shadow = dark;
      shadow.alpha = (int)(info_.shadow*0.25*255)/255.0f;
      layer(3, 3, shadow);
    }
    Brush fill;
    if (!allWhite && (info_.fill == 1 || info_.fill == 2)){
      //from the text's top over the font's height
      fill.kind = Brush::Gradient;
      fill.y0 = (float)y;
      fill.y1 = (float)(y+lineHeight_);
      fill.from = info_.fill == 1 ? 1.0f : 128.0f/255;
      fill.to = info_.fill == 1 ? 128.0f/255 : 1.0f;
    }
    else if (!allWhite && info_.fill == 3)
      fill.kind = Brush::Hatch;
    layer(0, 0, fill);
  }
  int textWidth(const std::string& text){
    int width = 0;
    for (char c : text)
      width += glyph((unsigned char)c).advance;
    return width + 2*padding_;
  }
  int lineHeight() const {return lineHeight_;}
  int padding() const {return padding_;}
  int outline() const {return outline_;}
  int shadow() const {return shadow_;}
private:
  FontInfo info_;
  std::vector<uint8_t> data_;
  FT_Library lib_ = nullptr;
  FT_Face face_ = nullptr;
  std::map<unsigned char, Glyph> glyphs_;
  int ascent_ = 0;
  int lineHeight_ = 0;
  int padding_ = 0;
  int outline_ = 0;
  int shadow_ = 0;
};

uint8_t toByte(float v){
  return (uint8_t)std::clamp((int)std::lround(v*255), 0, 255);
}

Bitmap toRgb(const Canvas& canvas){
  Bitmap bmp;
  bmp.width = canvas.width;
  bmp.height = canvas.height;
  bmp.channels = 3;
  bmp.pixels.resize((size_t)bmp.width*bmp.height*3);
  for (size_t i = 0, n = (size_t)bmp.width*bmp.height; i < n; ++i){
    for (int c = 0; c < 3; ++c)
      bmp.pixels[i*3+c] = toByte(canvas.rgba[i*4+c]);
  }
  return bmp;
}

}

bool renderBitmapFont(const FontInfo& info, BitmapFont& font, std::string* error){
  FontRenderer renderer;
  if (!renderer.load(info, error))
    return false;
  //the characters' widths and the largest one's extent (Fonts.createBitmapFont)
  int maxwidth = 0;
  font.widths.assign(kFontChars, 0);
  for (int i = 0; i < kFontChars; ++i){
    unsigned char ch = (unsigned char)(i+kFirstFontChar);
    const Glyph& g = renderer.glyph(ch);
    font.widths[i] = g.advance + renderer.outline() + info.spacing;
    maxwidth = std::max(maxwidth, renderer.padding() + std::max(g.advance, g.left+g.width));
  }
  int maxheight = renderer.lineHeight();
  if (maxheight == 0)
    maxheight = maxwidth;
  int charwidth = std::max(1, maxwidth + renderer.outline() + renderer.shadow());
  int charheight = maxheight + 2 + renderer.outline() + renderer.shadow();
  int perRow = std::max(1, 256/charwidth);
  int rows = std::max(1, 256/charheight);
  int perTexture = perRow*rows;
  int textures = (kFontChars + perTexture - 1)/perTexture;
  font.cellSize = Vec2i{charwidth, charheight};
  font.cells = Vec2i{perRow, rows};
  font.images.clear();
  font.alphaImages.clear();
  for (int t = 0; t < textures; ++t){
    Canvas color(256, 256, 0, 0, 0, 1);
    Canvas alpha(256, 256, 0, 0, 0, 1);
    for (int i = t*perTexture; i < std::min(kFontChars, (t+1)*perTexture); ++i){
      int row = i/perRow - t*rows;
      int column = i%perRow;
      std::string ch(1, (char)(i+kFirstFontChar));
      renderer.drawText(color, column*charwidth, row*charheight, ch, false);
      renderer.drawText(alpha, column*charwidth, row*charheight, ch, true);
    }
    font.images.push_back(toRgb(color));
    font.alphaImages.push_back(toRgb(alpha));
  }
  return true;
}

bool renderFontPreview(const FontInfo& info, const std::vector<std::string>& lines, int width, int height, Bitmap& image, std::string* error){
  FontRenderer renderer;
  if (!renderer.load(info, error))
    return false;
  Canvas canvas(width, height, 0, 0, 0, 0);
  //the Fonts dialog's lines: at the top, in the middle and at the bottom
  int count = (int)lines.size();
  for (int i = 0; i < count; ++i){
    int y = 0;
    if (count > 1)
      y = i*(height-renderer.lineHeight())/(count-1);
    int x = (width - renderer.textWidth(lines[i]))/2;
    renderer.drawText(canvas, x, y, lines[i], false);
  }
  image.width = width;
  image.height = height;
  image.channels = 4;
  image.pixels.resize((size_t)width*height*4);
  for (size_t i = 0, n = (size_t)width*height; i < n; ++i){
    float a = canvas.rgba[i*4+3];
    //the colours are painted over transparent black: undo the premultiplication
    for (int c = 0; c < 3; ++c)
      image.pixels[i*4+c] = toByte(a > 0 ? canvas.rgba[i*4+c]/a : 0);
    image.pixels[i*4+3] = toByte(a);
  }
  return true;
}

namespace{

void put16(std::string& out, uint32_t v){
  out += (char)(v & 0xFF);
  out += (char)((v >> 8) & 0xFF);
}

void put32(std::string& out, uint32_t v){
  put16(out, v & 0xFFFF);
  put16(out, v >> 16);
}

}

std::string writeBmp(const Bitmap& image){
  int rowSize = (image.width*3 + 3) & ~3;
  uint32_t dataSize = (uint32_t)rowSize*image.height;
  std::string out;
  out.reserve(54+dataSize);
  out += "BM";
  put32(out, 54+dataSize);
  put32(out, 0);
  put32(out, 54);
  put32(out, 40);
  put32(out, (uint32_t)image.width);
  put32(out, (uint32_t)image.height);
  put16(out, 1);
  put16(out, 24);
  put32(out, 0);
  put32(out, dataSize);
  //96 dots per inch, as .NET writes it
  put32(out, 3780);
  put32(out, 3780);
  put32(out, 0);
  put32(out, 0);
  //bottom-up rows of blue, green, red
  for (int y = image.height-1; y >= 0; --y){
    size_t start = out.size();
    for (int x = 0; x < image.width; ++x){
      const uint8_t* p = image.at(x, y);
      uint8_t r = p[0];
      uint8_t g = image.channels > 1 ? p[1] : p[0];
      uint8_t b = image.channels > 2 ? p[2] : p[0];
      out += (char)b;
      out += (char)g;
      out += (char)r;
    }
    out.append(rowSize - (out.size()-start), '\0');
  }
  return out;
}

std::string fontDataText(const BitmapFont& font){
  //StreamWriter.WriteLine's line ends
  std::string out;
  auto line = [&](int v){
    out += std::to_string(v);
    out += "\r\n";
  };
  line((int)font.images.size());
  line(font.cellSize.x);
  line(font.cellSize.y);
  line(font.cells.x);
  line(font.cells.y);
  for (int w : font.widths)
    line(w);
  return out;
}

std::string fontArchive(const BitmapFont& font, int number){
  CGE::ZipWriter zip;
  std::string num = std::to_string(number);
  for (size_t i = 0; i < font.images.size(); ++i){
    std::string bm = writeBmp(font.images[i]);
    zip.addEntry("font" + num + ".bm" + std::to_string(i+1), bm.data(), bm.size());
    std::string al = writeBmp(font.alphaImages[i]);
    zip.addEntry("font" + num + ".al" + std::to_string(i+1), al.data(), al.size());
  }
  //named as the game looks for it ("00" and the number; the C# one writes
  //three digits, which the game doesn't find from font 10 on)
  std::string data = fontDataText(font);
  zip.addEntry("fontdata.00" + num, data.data(), data.size());
  zip.close();
  return std::string(zip.getData().begin(), zip.getData().end());
}

}
