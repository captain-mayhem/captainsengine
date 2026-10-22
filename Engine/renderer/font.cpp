//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: font.cpp                                                   |
//  ==================================================================

#include "../system/engine.h"
#include "../platform/assets.h"
#include "font.h"
#include "drawcontext.h"
#include "renderdevice.h"
#include "textureloader.h"

#include <cstdint>

using std::string;
using std::ostringstream;
using std::queue;

using namespace CGE;

TR_CHANNEL(CGE_Font);

namespace{

//! a glyph corner
struct GlyphVertex{
  float position[3];
  uint8_t color[4];
  float texcoord[2];
};

const Gfx::VertexLayout& glyphLayout(){
  using namespace Gfx;
  static const VertexLayout layout = {
    {
      {ATTR_POSITION, VertexFormat::Float3, 0, 0},
      {ATTR_COLOR, VertexFormat::UByte4Norm, 0, 12},
      {ATTR_TEXCOORD, VertexFormat::Float2, 0, 16},
    },
    {{(int)sizeof(GlyphVertex), false}},
  };
  return layout;
}

uint8_t toByte(float value){
  if (value <= 0)
    return 0;
  if (value >= 1)
    return 255;
  return (uint8_t)(value*255.0f+0.5f);
}

}

//Constructor
Font::Font(Gfx::RenderDevice& device){
	TR_USE(CGE_Font);
  show_ = true;
  rgb_ = Color(1.0,1.0,1.0,1.0);
  id_ = -1;
  const char* file = "textures/font/font.bmp";
  if (Assets::exists(file))
    tex_ = Gfx::loadTexture(device, file);
  if (tex_ == nullptr){
  	TR_WARN("Builtin font  not available");
    return;
  }
  //like the old renderers' textures: linear with mipmaps, repeating
  Gfx::SamplerDesc sampler;
  sampler.mipFilter = Gfx::MipFilter::Linear;
  sampler_ = device.createSampler(sampler);
}

//Destructor
Font::~Font(){
  clear();
}

// Stores all necessary information to render the text
void Font::print(int x, int y, const char* str, short set, float duration){
  font_data f;
  f.pos.x = x;
  f.pos.y = y;
  f.set = set;
  f.text = new string(str);
  f.rgb = rgb_;
  f.duration = duration;
  f.id = id_;
  q_.push(f);
  id_ = -1;
}

// Set font color
void Font::setColor(float r, float g, float b){
  rgb_.r = r;
  rgb_.g = g;
  rgb_.b = b;
}

// Set font color
void Font::setColor(Color& c){
  rgb_ = c;
}

// Render all stored text strings
void Font::render(Gfx::DrawContext& ctx, double interval){
  queue<font_data> temp;
  std::vector<GlyphVertex> vertices;

  while(!q_.empty()){
    
    //take out the next font_data
    font_data f = q_.front();
    q_.pop();
    f.duration -= interval;
    
    //not displayed long enough, so display again in the next cycle
    if (f.duration > 0){
      temp.push(f);
    }
    if (show_ && tex_){
      if (f.set>1){
        f.set=1;
      }
      uint8_t color[4] = {toByte(f.rgb.r), toByte(f.rgb.g), toByte(f.rgb.b), toByte(f.rgb.a)};
      float x = (float)f.pos.x;
      float y = (float)f.pos.y;
      for (unsigned i = 0; i < f.text->size(); i++){
        //the font has 256 glyphs: two sets of 128 characters from the space on
        int glyph = (int)(unsigned char)f.text->at(i)-32+(128*f.set);
        if (glyph >= 0 && glyph < 256){
          //position of the character in the font texture, 16x16 units on the screen
          float cx = float(glyph % 16) / 16.0f;
          float cy = float(glyph / 16) / 16.0f;
          GlyphVertex bottomLeft = {{x, y, 0}, {color[0], color[1], color[2], color[3]}, {cx, cy + 0.0625f}};
          GlyphVertex bottomRight = {{x + 16, y, 0}, {color[0], color[1], color[2], color[3]}, {cx + 0.0625f, cy + 0.0625f}};
          GlyphVertex topLeft = {{x, y + 16, 0}, {color[0], color[1], color[2], color[3]}, {cx, cy}};
          GlyphVertex topRight = {{x + 16, y + 16, 0}, {color[0], color[1], color[2], color[3]}, {cx + 0.0625f, cy}};
          vertices.insert(vertices.end(), {bottomLeft, bottomRight, topLeft, topLeft, bottomRight, topRight});
        }
        x += 10;
      }
    }
    if (f.duration <= 0)
      delete f.text;
  }
  q_ = temp;

  if (vertices.empty())
    return;
  Gfx::RenderDevice& device = ctx.getDevice();
  size_t size = vertices.size()*sizeof(GlyphVertex);
  //a buffer for the longest text so far, whole-buffer updates
  if (!buffer_ || buffer_->getDesc().size < size){
    buffer_ = device.createBuffer({Gfx::BufferType::Vertex, Gfx::Usage::Stream, size});
    if (!buffer_)
      return;
  }
  device.updateBuffer(buffer_.get(), 0, size, vertices.data());
  ctx.setTexture(tex_.get(), sampler_.get());
  ctx.setColor(1, 1, 1, 1);
  ctx.draw(Gfx::BuiltinShader::Textured, buffer_.get(), glyphLayout(), Gfx::PrimitiveType::Triangles, (int)vertices.size());
  ctx.setTexture(nullptr);
}

//simplified glPrint
void Font::operator<<(const string& text){
  clear();
  setColor(1.0,1.0,1.0);
  print(20, 750, text.c_str(), 0, 10);
}

//for stringstreams
void Font::operator<<(const ostringstream& text){
  (*this) << text.str();
}

// delete the texts with a certain id
void Font::deleteText(short id){
  queue<font_data> temp;
  
  while(!q_.empty()){ 
    font_data f = q_.front();
    q_.pop();
    if (f.id != id)
      temp.push(f);
    else
      delete f.text;
  }
  q_ = temp;
}