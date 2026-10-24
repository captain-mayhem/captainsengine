#include "font.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <unordered_map>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_STROKER_H
#include FT_SYNTHESIS_H

#include "drawcontext.h"
#include "renderdevice.h"
#include "spritebatch.h"
#include "../io/Tracing.h"
#include "../math/matrix.h"
#include "../platform/assets.h"
#include "../platform/systemfonts.h"

using namespace CGE;

TR_CHANNEL(CGE_Font);

namespace{

//! the atlas textures' width and height
const int ATLAS_SIZE = 1024;
//! with this many pages, they are emptied when a new frame begins
const int MAX_PAGES = 4;
//! empty pixels between glyphs, so filtering doesn't pick up a neighbour
const int PADDING = 1;
//! distance field glyphs are rendered at this many pixels per em
const int DISTANCE_SIZE = 48;
//! with distances up to this many pixels on either side of the edge (FreeType's default)
const int DISTANCE_SPREAD = 8;

//! FreeType, shared by the fonts
FT_Library library = nullptr;
int libraryUsers = 0;

FT_Library acquireLibrary(){
  if (!library && FT_Init_FreeType(&library) != 0){
    library = nullptr;
    return nullptr;
  }
  ++libraryUsers;
  return library;
}

void releaseLibrary(){
  if (--libraryUsers == 0){
    FT_Done_FreeType(library);
    library = nullptr;
  }
}

//! a glyph at a size (26.6 pixels) with an outline (26.6 pixels, 0 for the glyph itself)
struct GlyphKey{
  FT_UInt index;
  int size;
  int outline;
  bool operator==(const GlyphKey& other) const{
    return index == other.index && size == other.size && outline == other.outline;
  }
};

struct GlyphKeyHash{
  size_t operator()(const GlyphKey& key) const{
    return std::hash<uint64_t>()(((uint64_t)key.index << 40) ^ ((uint64_t)key.size << 20) ^ (uint64_t)key.outline);
  }
};

//! a rendered glyph: its bitmap in a page, placed relative to the pen on the baseline (pixels, y up)
struct Glyph{
  //! -1 if it has no pixels (a space)
  int page = -1;
  int left = 0;
  int top = 0;
  int width = 0;
  int height = 0;
  Gfx::TexRect rect;
};

//! an atlas texture, filled row by row, and its pixels
struct Page{
  std::unique_ptr<Gfx::Texture> texture;
  std::vector<uint8> pixels;
  int x = PADDING;
  int y = PADDING;
  int rowHeight = 0;
  //! the rows changed since the last upload
  int dirtyMin = ATLAS_SIZE;
  int dirtyMax = 0;
};

//! a glyph laid out: its pen position on its line (26.6 pixels)
struct Placed{
  FT_UInt index;
  int x;
  int line;
};

struct Layout{
  std::vector<Placed> glyphs;
  //! each line's width (26.6 pixels)
  std::vector<int> widths;
};

}

struct Font::Impl{
  ~Impl(){
    pages.clear();
    if (stroker)
      FT_Stroker_Done(stroker);
    if (face)
      FT_Done_Face(face);
    if (ft)
      releaseLibrary();
  }

  bool setSize(int size){
    if (size == currentSize)
      return true;
    if (FT_Set_Char_Size(face, 0, size, 72, 72) != 0)
      return false;
    currentSize = size;
    return true;
  }

  FT_UInt getIndex(char32_t ch){
    auto it = indices.find(ch);
    if (it != indices.end())
      return it->second;
    FT_UInt index = FT_Get_Char_Index(face, ch);
    indices[ch] = index;
    return index;
  }

  //! loads a glyph into the face's slot, with the synthetic styles
  bool loadGlyph(FT_UInt index, int size){
    //distance fields scale, so their outlines aren't fitted to one size's pixels
    FT_Int32 flags = FT_LOAD_NO_BITMAP | (desc.distanceField ? FT_LOAD_NO_HINTING : FT_LOAD_TARGET_LIGHT);
    if (!setSize(size) || FT_Load_Glyph(face, index, flags) != 0)
      return false;
    if (desc.oblique)
      FT_GlyphSlot_Oblique(face->glyph);
    if (desc.embolden)
      FT_GlyphSlot_Embolden(face->glyph);
    return true;
  }

  //! 26.6 pixels
  int getAdvance(FT_UInt index, int size){
    uint64_t key = ((uint64_t)index << 32) | (uint32_t)size;
    auto it = advances.find(key);
    if (it != advances.end())
      return it->second;
    int advance = loadGlyph(index, size) ? (int)face->glyph->advance.x : 0;
    advances[key] = advance;
    return advance;
  }

  //! 26.6 pixels
  int getKerning(FT_UInt left, FT_UInt right, int size){
    if (!FT_HAS_KERNING(face) || !setSize(size))
      return 0;
    FT_Vector delta;
    if (FT_Get_Kerning(face, left, right, desc.distanceField ? FT_KERNING_UNFITTED : FT_KERNING_DEFAULT, &delta) != 0)
      return 0;
    return (int)delta.x;
  }

  //! the lines and the glyphs' pen positions; lines wider than wrap (26.6 pixels, 0: no wrapping) are wrapped at spaces
  Layout layout(std::string_view text, int size, int wrap){
    Layout result;
    int pen = 0;
    int line = 0;
    FT_UInt previous = 0;
    //the last space on the line, where it can be wrapped
    int space = -1;
    size_t pos = 0;
    while (pos < text.size()){
      char32_t ch = nextCharacter(text, pos);
      if (ch == '\n'){
        result.widths.push_back(pen);
        ++line;
        pen = 0;
        previous = 0;
        space = -1;
        continue;
      }
      if (ch == '\r')
        continue;
      bool blank = ch == ' ' || ch == '\t';
      FT_UInt index = getIndex(ch == '\t' ? U' ' : ch);
      if (previous)
        pen += getKerning(previous, index, size);
      int advance = getAdvance(index, size)*(ch == '\t' ? 4 : 1);
      if (wrap > 0 && !blank && pen+advance > wrap && space >= 0){
        //the words after the space go onto the next line
        int shift = space+1 < (int)result.glyphs.size() ? result.glyphs[space+1].x : pen;
        result.widths.push_back(result.glyphs[space].x);
        ++line;
        for (size_t i = space+1; i < result.glyphs.size(); ++i){
          result.glyphs[i].x -= shift;
          result.glyphs[i].line = line;
        }
        pen -= shift;
        space = -1;
      }
      result.glyphs.push_back({index, pen, line});
      if (blank)
        space = (int)result.glyphs.size()-1;
      pen += advance;
      previous = index;
    }
    result.widths.push_back(pen);
    return result;
  }

  //! where a w x h bitmap goes into the atlas
  bool allocate(int width, int height, int& page, int& x, int& y){
    if (width+2*PADDING > ATLAS_SIZE || height+2*PADDING > ATLAS_SIZE)
      return false;
    if (!pages.empty()){
      Page& last = pages.back();
      if (last.x+width+PADDING > ATLAS_SIZE){
        last.x = PADDING;
        last.y += last.rowHeight+PADDING;
        last.rowHeight = 0;
      }
    }
    if (pages.empty() || pages.back().y+height+PADDING > ATLAS_SIZE){
      Page next;
      //white, so filtering at the glyphs' edges keeps their colour
      next.pixels.resize(ATLAS_SIZE*ATLAS_SIZE*4);
      for (size_t i = 0; i < next.pixels.size(); i += 4){
        next.pixels[i] = next.pixels[i+1] = next.pixels[i+2] = 255;
        next.pixels[i+3] = 0;
      }
      Gfx::TextureDesc tex;
      tex.width = tex.height = ATLAS_SIZE;
      next.texture = device->createTexture(tex, next.pixels.data());
      if (!next.texture)
        return false;
      pages.push_back(std::move(next));
    }
    Page& last = pages.back();
    page = (int)pages.size()-1;
    x = last.x;
    y = last.y;
    last.x += width+PADDING;
    last.rowHeight = (std::max)(last.rowHeight, height);
    last.dirtyMin = (std::min)(last.dirtyMin, y);
    last.dirtyMax = (std::max)(last.dirtyMax, y+height);
    return true;
  }

  //! the glyph, rendered into the atlas if it isn't yet
  const Glyph& render(FT_UInt index, int size, int outline){
    TR_USE(CGE_Font);
    GlyphKey key = {index, size, outline};
    auto it = glyphs.find(key);
    if (it != glyphs.end())
      return it->second;
    Glyph& result = glyphs[key];
    FT_Glyph glyph = nullptr;
    if (!loadGlyph(index, size) || FT_Get_Glyph(face->glyph, &glyph) != 0)
      return result;
    if (outline > 0 && glyph->format == FT_GLYPH_FORMAT_OUTLINE){
      //the border outside the glyph, filled: the glyph grown by the outline's width
      FT_Stroker_Set(stroker, outline, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
      FT_Glyph_StrokeBorder(&glyph, stroker, 0, 1);
    }
    if (FT_Glyph_To_Bitmap(&glyph, desc.distanceField ? FT_RENDER_MODE_SDF : FT_RENDER_MODE_NORMAL, nullptr, 1) != 0){
      FT_Done_Glyph(glyph);
      return result;
    }
    FT_BitmapGlyph bitmapGlyph = (FT_BitmapGlyph)glyph;
    const FT_Bitmap& bitmap = bitmapGlyph->bitmap;
    int width = (int)bitmap.width;
    int height = (int)bitmap.rows;
    int page, x, y;
    if (width > 0 && height > 0 && bitmap.pixel_mode == FT_PIXEL_MODE_GRAY){
      if (allocate(width, height, page, x, y)){
        Page& target = pages[page];
        for (int row = 0; row < height; ++row){
          const uint8* src = bitmap.buffer + row*bitmap.pitch;
          uint8* dst = &target.pixels[((y+row)*ATLAS_SIZE + x)*4];
          for (int col = 0; col < width; ++col)
            dst[col*4+3] = src[col];
        }
        result.page = page;
        result.left = bitmapGlyph->left;
        result.top = bitmapGlyph->top;
        result.width = width;
        result.height = height;
        result.rect.u0 = (float)x/ATLAS_SIZE;
        result.rect.v0 = (float)y/ATLAS_SIZE;
        result.rect.u1 = (float)(x+width)/ATLAS_SIZE;
        result.rect.v1 = (float)(y+height)/ATLAS_SIZE;
      }
      else
        TR_ERROR("glyph %u of %s doesn't fit into the atlas", index, family.c_str());
    }
    FT_Done_Glyph(glyph);
    return result;
  }

  //! the rows rendered since the last upload go into the textures
  void upload(){
    for (Page& page : pages){
      if (page.dirtyMax <= page.dirtyMin)
        continue;
      Gfx::TextureRegion region;
      region.y = page.dirtyMin;
      region.width = ATLAS_SIZE;
      region.height = page.dirtyMax-page.dirtyMin;
      device->updateTexture(page.texture.get(), region, &page.pixels[page.dirtyMin*ATLAS_SIZE*4]);
      page.dirtyMin = ATLAS_SIZE;
      page.dirtyMax = 0;
    }
  }

  //! a full atlas is emptied when a new frame begins, when no sprite waits for its textures
  void checkFrame(){
    uint64_t current = device->getFrameCount();
    if (current == frame)
      return;
    frame = current;
    if ((int)pages.size() >= MAX_PAGES){
      glyphs.clear();
      pages.clear();
    }
  }

  //! in units per em, scaled to size
  float scaled(FT_Short value, float size) const{
    return (float)value*size/face->units_per_EM;
  }

  float sizeOf(const TextStyle& style) const{
    return style.size > 0 ? style.size : desc.size;
  }

  Gfx::RenderDevice* device = nullptr;
  FontDesc desc;
  std::vector<uint8> data;
  std::string family;
  FT_Library ft = nullptr;
  FT_Face face = nullptr;
  FT_Stroker stroker = nullptr;
  int currentSize = 0;
  std::unordered_map<char32_t, FT_UInt> indices;
  std::unordered_map<uint64_t, int> advances;
  std::unordered_map<GlyphKey, Glyph, GlyphKeyHash> glyphs;
  std::vector<Page> pages;
  std::unique_ptr<Gfx::Sampler> sampler;
  uint64_t frame = 0;
};

Font::Font() : impl_(std::make_unique<Impl>()){
}

Font::~Font(){
}

std::unique_ptr<Font> Font::load(Gfx::RenderDevice& device, std::vector<uint8> data, const FontDesc& desc){
  TR_USE(CGE_Font);
  std::unique_ptr<Font> font(new Font());
  Impl& impl = *font->impl_;
  impl.device = &device;
  impl.desc = desc;
  if (impl.desc.size <= 0)
    impl.desc.size = 16;
  impl.data = std::move(data);
  impl.ft = acquireLibrary();
  if (!impl.ft){
    TR_ERROR("FreeType can't be initialised");
    return nullptr;
  }
  if (FT_New_Memory_Face(impl.ft, impl.data.data(), (FT_Long)impl.data.size(), desc.faceIndex, &impl.face) != 0){
    TR_ERROR("not a font file");
    return nullptr;
  }
  if (!FT_IS_SCALABLE(impl.face)){
    TR_ERROR("%s isn't a scalable font", impl.face->family_name ? impl.face->family_name : "the font");
    return nullptr;
  }
  FT_Select_Charmap(impl.face, FT_ENCODING_UNICODE);
  impl.family = impl.face->family_name ? impl.face->family_name : "";
  if (FT_Stroker_New(impl.ft, &impl.stroker) != 0)
    impl.stroker = nullptr;
  Gfx::SamplerDesc sampler;
  sampler.addressU = sampler.addressV = sampler.addressW = Gfx::AddressMode::ClampToEdge;
  impl.sampler = device.createSampler(sampler);
  return font;
}

std::unique_ptr<Font> Font::load(Gfx::RenderDevice& device, const std::string& path, const FontDesc& desc){
  TR_USE(CGE_Font);
  std::vector<uint8> data;
  if (path.empty()){
    if (!loadDefaultData(data)){
      TR_ERROR("neither %s nor a system font can be loaded", getDefaultPath());
      return nullptr;
    }
  }
  else if (!Assets::load(path, data)){
    TR_ERROR("%s can't be loaded", path.c_str());
    return nullptr;
  }
  return load(device, std::move(data), desc);
}

std::unique_ptr<Font> Font::loadSystem(Gfx::RenderDevice& device, const std::string& family, const FontDesc& desc, int weight, bool italic){
  TR_USE(CGE_Font);
  SystemFonts::FontData font;
  if (!SystemFonts::find(family, font, weight, italic)){
    TR_ERROR("no system font for %s", family.c_str());
    return nullptr;
  }
  FontDesc faceDesc = desc;
  faceDesc.faceIndex = font.faceIndex;
  return load(device, std::move(font.data), faceDesc);
}

const char* Font::getDefaultPath(){
  return "fonts/DejaVuSans.ttf";
}

bool Font::loadDefaultData(std::vector<uint8>& data){
  if (Assets::exists(getDefaultPath()) && Assets::load(getDefaultPath(), data) && !data.empty())
    return true;
  SystemFonts::FontData font;
  //a collection's other faces than the first would need its index
  if (SystemFonts::find("sans-serif", font) && font.faceIndex == 0){
    data = std::move(font.data);
    return true;
  }
  return false;
}

const std::string& Font::getFamily() const{
  return impl_->family;
}

const FontDesc& Font::getDesc() const{
  return impl_->desc;
}

float Font::getAscent(float size) const{
  return impl_->scaled(impl_->face->ascender, size > 0 ? size : impl_->desc.size);
}

float Font::getDescent(float size) const{
  return -impl_->scaled(impl_->face->descender, size > 0 ? size : impl_->desc.size);
}

float Font::getLineHeight(float size) const{
  return impl_->scaled(impl_->face->height, size > 0 ? size : impl_->desc.size);
}

bool Font::hasGlyph(char32_t ch) const{
  return impl_->getIndex(ch) != 0;
}

Vec2f Font::measure(std::string_view text, const TextStyle& style) const{
  float size = impl_->sizeOf(style);
  //at one pixel per unit
  int pixels = impl_->desc.distanceField ? DISTANCE_SIZE*64 : (std::max)(1, (int)std::lround(size*64));
  float units = size/(pixels/64.0f);
  Layout layout = impl_->layout(text, pixels, style.wrapWidth > 0 ? (int)(style.wrapWidth/units*64) : 0);
  int widest = 0;
  for (int width : layout.widths)
    widest = (std::max)(widest, width);
  float lines = (float)layout.widths.size();
  return Vec2f(widest/64.0f*units, (lines-1)*getLineHeight(size) + getAscent(size) + getDescent(size));
}

void Font::draw(Gfx::SpriteBatch& batch, float x, float y, std::string_view text, const TextStyle& style){
  Impl& impl = *impl_;
  impl.checkFrame();
  float size = impl.sizeOf(style);
  //the screen's pixels per unit, from the projection and the viewport
  Gfx::DrawContext& ctx = batch.getContext();
  Matrix transform = ctx.getProjection()*ctx.getView();
  const float* m = transform.getData();
  const Gfx::Viewport& viewport = impl.device->getViewport();
  float scale = std::sqrt(m[0]*m[0] + m[1]*m[1])*viewport.width*0.5f;
  if (!(scale > 0) || !std::isfinite(scale))
    scale = 1;
  //the glyphs' size in pixels, and units per pixel of their bitmaps
  int pixels = impl.desc.distanceField ? DISTANCE_SIZE*64 : (std::max)(1, (int)std::lround(size*scale*64));
  float units = size/(pixels/64.0f);
  Layout layout = impl.layout(text, pixels, style.wrapWidth > 0 ? (int)(style.wrapWidth/units*64) : 0);
  if (layout.glyphs.empty())
    return;

  //the shadow (of the outline, if there is one), the outline and the glyphs;
  //normal glyphs have an outline of their own, distance fields a lower edge
  struct Layer{
    int outline;
    float edge;
    Color color;
    float dx;
    float dy;
  };
  bool outlined = style.outline > 0 && (impl.stroker || impl.desc.distanceField);
  int outline = outlined && !impl.desc.distanceField ? (std::max)(1, (int)std::lround(style.outline/units*64)) : 0;
  float fillEdge = impl.desc.distanceField ? 0.5f : -1;
  //a distance field's values go from 0 to 1 over twice the spread
  float outlineEdge = impl.desc.distanceField ? (std::max)(0.5f - style.outline/units*0.5f/DISTANCE_SPREAD, 0.02f) : -1;
  std::vector<Layer> layers;
  if (style.shadowX != 0 || style.shadowY != 0){
    if (outlined)
      layers.push_back({outline, outlineEdge, style.shadowColor, style.shadowX, style.shadowY});
    else
      layers.push_back({0, fillEdge, style.shadowColor, style.shadowX, style.shadowY});
  }
  if (outlined)
    layers.push_back({outline, outlineEdge, style.outlineColor, 0, 0});
  layers.push_back({0, fillEdge, style.color, 0, 0});

  //all glyphs into the atlas first, so its textures are updated once
  for (const Layer& layer : layers){
    for (const Placed& placed : layout.glyphs)
      impl.render(placed.index, pixels, layer.outline);
  }
  impl.upload();

  //normal glyphs on whole pixels, where the projection is axis-aligned
  float snapX = 0;
  float snapY = 0;
  float ascent = getAscent(size);
  float lineHeight = getLineHeight(size);
  bool snap = !impl.desc.distanceField && m[1] == 0 && m[4] == 0 && m[0] != 0 && m[5] != 0;
  if (snap){
    float baseline = y + ascent;
    float px = (m[0]*x + m[12] + 1)*0.5f*viewport.width;
    float py = (m[5]*baseline + m[13] + 1)*0.5f*viewport.height;
    snapX = (std::round(px)-px)/(m[0]*0.5f*viewport.width);
    snapY = (std::round(py)-py)/(m[5]*0.5f*viewport.height);
  }

  Gfx::Sampler* oldSampler = batch.getSampler();
  float oldEdge = batch.getDistanceField();
  batch.setSampler(impl.sampler.get());
  for (const Layer& layer : layers){
    batch.setDistanceField(layer.edge);
    for (const Placed& placed : layout.glyphs){
      const Glyph& glyph = impl.render(placed.index, pixels, layer.outline);
      if (glyph.page < 0)
        continue;
      //whole pixels from the line's start, which may be in the middle of one
      int lineWidth = layout.widths[placed.line];
      int offset = style.align == TextAlign::Center ? lineWidth/2 : style.align == TextAlign::Right ? lineWidth : 0;
      if (snap)
        offset = (offset+32) & ~63;
      float left = x + snapX + (placed.x - offset)/64.0f*units + glyph.left*units + layer.dx;
      float top = y + snapY + ascent + placed.line*lineHeight - glyph.top*units + layer.dy;
      batch.draw(impl.pages[glyph.page].texture.get(), left, top, glyph.width*units, glyph.height*units, glyph.rect, layer.color);
    }
  }
  batch.setSampler(oldSampler);
  batch.setDistanceField(oldEdge);
}

Font::Stats Font::getStats() const{
  Stats stats;
  for (const auto& entry : impl_->glyphs){
    if (entry.second.page >= 0)
      ++stats.glyphs;
  }
  stats.pages = (int)impl_->pages.size();
  return stats;
}

char32_t CGE::nextCharacter(std::string_view text, size_t& pos){
  unsigned char lead = (unsigned char)text[pos];
  int extra = 0;
  if (lead >= 0xC2 && lead <= 0xDF)
    extra = 1;
  else if (lead >= 0xE0 && lead <= 0xEF)
    extra = 2;
  else if (lead >= 0xF0 && lead <= 0xF4)
    extra = 3;
  //ASCII, or not the start of a whole sequence (ISO 8859-1)
  if (extra == 0 || pos+extra >= text.size()){
    ++pos;
    return lead;
  }
  char32_t ch = lead & (0x3F >> extra);
  for (int i = 1; i <= extra; ++i){
    unsigned char next = (unsigned char)text[pos+i];
    if ((next & 0xC0) != 0x80){
      ++pos;
      return lead;
    }
    ch = (ch << 6) | (next & 0x3F);
  }
  //overlong sequences, surrogates and beyond Unicode aren't UTF-8 either
  if ((extra == 2 && ch < 0x800) || (extra == 3 && ch < 0x10000) || (ch >= 0xD800 && ch <= 0xDFFF) || ch > 0x10FFFF){
    ++pos;
    return lead;
  }
  pos += extra+1;
  return ch;
}
