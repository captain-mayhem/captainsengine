#include "spritebatch.h"

#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

#include "renderdevice.h"
#include "shaderlibrary.h"
#include "textureatlas.h"
#include "../io/Tracing.h"
#include "../math/matrix.h"

using namespace CGE;
using namespace CGE::Gfx;

TR_CHANNEL(CGE_SpriteBatch);

namespace{

//! the sprite shader's per-instance inputs (shaders/sprite.vert)
const int ATTR_AXES = 4;
const int ATTR_ORIGIN = 5;
const int ATTR_TEXRECT = 6;

//! one sprite: the unit quad's corner (0, 0) goes to origin, (1, 0) to
//! origin+axisX and (0, 1) to origin+axisY
struct Instance{
  float axes[4];
  float origin[3];
  float texrect[4];
  uint8_t color[4];
};
static_assert(sizeof(Instance) == 48, "Instance must be tightly packed");

uint8_t toByte(float value){
  if (value <= 0)
    return 0;
  if (value >= 1)
    return 255;
  return (uint8_t)(value*255.0f+0.5f);
}

//! a sprite's placement before the texture coordinates and colour
struct Placement{
  float ax, ay;
  float bx, by;
  float ox, oy, oz;
};

//! the unit quad onto the sprite's rectangle; the part from (s0, t0) to (s1, t1) of it
Placement place(const Sprite& sprite, float s0, float t0, float s1, float t1){
  float w = sprite.width*sprite.scaleX;
  float h = sprite.height*sprite.scaleY;
  float angle = sprite.rotation/180.0f*3.14159265358979f;
  float c = std::cos(angle);
  float s = std::sin(angle);
  //the quad's axes after scaling and rotating
  float ax = w*c, ay = w*s;
  float bx = -h*s, by = h*c;
  //its corner (0, 0), with the origin at (x, y)
  float ox = sprite.x - sprite.originX*ax - sprite.originY*bx;
  float oy = sprite.y - sprite.originX*ay - sprite.originY*by;
  Placement p;
  p.ox = ox + s0*ax + t0*bx;
  p.oy = oy + s0*ay + t0*by;
  p.oz = sprite.depth;
  p.ax = ax*(s1-s0);
  p.ay = ay*(s1-s0);
  p.bx = bx*(t1-t0);
  p.by = by*(t1-t0);
  return p;
}

}

struct SpriteBatch::Impl{
  Impl(DrawContext& ctx, int cap) : context(ctx), device(ctx.getDevice()), capacity(cap < 1 ? 1 : cap){
    pending.reserve(capacity);
  }

  //! the sprites wait for a draw with this texture, sampler and blend mode
  void add(Texture* tex, const Instance& instance){
    checkFrame();
    if (!pending.empty() && (tex != pendingTexture || sampler != pendingSampler || blend != pendingBlend ||
        edge != pendingEdge || (int)pending.size() >= capacity))
      flush();
    if (pending.empty()){
      pendingTexture = tex;
      pendingSampler = sampler;
      pendingBlend = blend;
      pendingEdge = edge;
      pendingFrame = device.getFrameCount();
    }
    pending.push_back(instance);
  }

  void addPlacement(Texture* tex, const Placement& p, const TexRect& rect, const Color& color){
    Instance instance;
    instance.axes[0] = p.ax;
    instance.axes[1] = p.ay;
    instance.axes[2] = p.bx;
    instance.axes[3] = p.by;
    instance.origin[0] = p.ox;
    instance.origin[1] = p.oy;
    instance.origin[2] = p.oz;
    instance.texrect[0] = rect.u0;
    instance.texrect[1] = rect.v0;
    instance.texrect[2] = rect.u1;
    instance.texrect[3] = rect.v1;
    instance.color[0] = toByte(color.r);
    instance.color[1] = toByte(color.g);
    instance.color[2] = toByte(color.b);
    instance.color[3] = toByte(color.a);
    add(tex, instance);
  }

  //! drops what is left of the last frame and starts using the buffers anew
  void checkFrame(){
    TR_USE(CGE_SpriteBatch);
    uint64_t current = device.getFrameCount();
    if (!pending.empty() && pendingFrame != current){
      TR_ERROR("%d sprites weren't flushed in their frame and are dropped", (int)pending.size());
      pending.clear();
    }
    if (frame != current){
      frame = current;
      block = 0;
      used = 0;
      stats = Stats();
    }
  }

  bool createResources(){
    if (!quad){
      //the unit quad as a triangle strip
      const float corners[8] = {0, 0, 1, 0, 0, 1, 1, 1};
      BufferDesc desc;
      desc.type = BufferType::Vertex;
      desc.size = sizeof(corners);
      quad = device.createBuffer(desc, corners);
    }
    if (!white){
      TextureDesc desc;
      desc.width = desc.height = 1;
      const uint8_t pixel[4] = {255, 255, 255, 255};
      white = device.createTexture(desc, pixel);
    }
    if (!defaultSampler){
      SamplerDesc desc;
      desc.mipFilter = MipFilter::Linear;
      desc.addressU = desc.addressV = desc.addressW = AddressMode::ClampToEdge;
      defaultSampler = device.createSampler(desc);
    }
    if (!shader)
      shader = ShaderLibrary::createBuiltin(device, BuiltinShader::Sprite);
    if (pendingEdge >= 0 && !distanceShader)
      distanceShader = ShaderLibrary::createBuiltin(device, BuiltinShader::SpriteDistanceField);
    return quad && white && defaultSampler && shader && (pendingEdge < 0 || distanceShader);
  }

  //! a range of the instance buffers for count instances, written with the data
  bool upload(const Instance* data, int count, Buffer*& buffer, size_t& offset){
    if (used+count > capacity){
      ++block;
      used = 0;
    }
    if (block >= (int)blocks.size()){
      BufferDesc desc;
      desc.type = BufferType::Vertex;
      desc.usage = Usage::Stream;
      desc.size = capacity*sizeof(Instance);
      std::unique_ptr<Buffer> next = device.createBuffer(desc);
      if (!next)
        return false;
      blocks.push_back(std::move(next));
    }
    buffer = blocks[block].get();
    offset = used*sizeof(Instance);
    device.updateBuffer(buffer, offset, count*sizeof(Instance), data);
    used += count;
    return true;
  }

  void flush(){
    TR_USE(CGE_SpriteBatch);
    checkFrame();
    if (pending.empty())
      return;
    int count = (int)pending.size();
    if (!device.isInPass()){
      TR_ERROR("flush outside of a pass: %d sprites are dropped", count);
      pending.clear();
      return;
    }
    Buffer* buffer = nullptr;
    size_t offset = 0;
    if (!createResources() || !upload(pending.data(), count, buffer, offset)){
      TR_ERROR("cannot create the sprite batch's resources");
      pending.clear();
      return;
    }
    pending.clear();

    //the context's own state stays as it was
    Texture* oldTexture = context.getTexture();
    Sampler* oldSampler = context.getSampler();
    BlendMode oldBlend = context.getBlend();
    CullMode oldCull = context.getCull();
    Color oldColor = context.getColor();
    context.setTexture(pendingTexture ? pendingTexture : white.get(), pendingSampler ? pendingSampler : defaultSampler.get());
    context.setBlend(pendingBlend);
    context.setCull(CullMode::Off);
    //the distance field shader takes the edge from the draw's colour, which the sprites don't use
    Shader* drawShader = shader.get();
    if (pendingEdge >= 0){
      drawShader = distanceShader.get();
      context.setColor(pendingEdge, 0, 0, 0);
    }
    if (context.prepare(drawShader, getLayout(), PrimitiveType::TriangleStrip)){
      device.setVertexBuffer(0, quad.get());
      device.setVertexBuffer(1, buffer, offset);
      device.draw(4, count);
      ++stats.draws;
      stats.sprites += count;
    }
    context.setTexture(oldTexture, oldSampler);
    context.setBlend(oldBlend);
    context.setCull(oldCull);
    context.setColor(oldColor);
  }

  static const VertexLayout& getLayout(){
    static VertexLayout layout;
    if (layout.attributes.empty()){
      layout.attributes = {
        {ATTR_POSITION, VertexFormat::Float2, 0, 0},
        {ATTR_AXES, VertexFormat::Float4, 1, (int)offsetof(Instance, axes)},
        {ATTR_ORIGIN, VertexFormat::Float3, 1, (int)offsetof(Instance, origin)},
        {ATTR_TEXRECT, VertexFormat::Float4, 1, (int)offsetof(Instance, texrect)},
        {ATTR_COLOR, VertexFormat::UByte4Norm, 1, (int)offsetof(Instance, color)},
      };
      VertexBufferLayout corners;
      corners.stride = 2*sizeof(float);
      VertexBufferLayout instances;
      instances.stride = sizeof(Instance);
      instances.perInstance = true;
      layout.buffers = {corners, instances};
    }
    return layout;
  }

  DrawContext& context;
  RenderDevice& device;
  int capacity;

  BlendMode blend = BlendMode::Alpha;
  Sampler* sampler = nullptr;
  //! the distance field's edge, negative for normal sprites
  float edge = -1;

  std::vector<Instance> pending;
  Texture* pendingTexture = nullptr;
  Sampler* pendingSampler = nullptr;
  BlendMode pendingBlend = BlendMode::Alpha;
  float pendingEdge = -1;
  uint64_t pendingFrame = 0;

  //! the instance buffers of capacity instances each; the one being filled and its instances
  std::vector<std::unique_ptr<Buffer>> blocks;
  int block = 0;
  int used = 0;
  uint64_t frame = 0;
  Stats stats;

  std::unique_ptr<Buffer> quad;
  std::unique_ptr<Texture> white;
  std::unique_ptr<Sampler> defaultSampler;
  std::unique_ptr<Shader> shader;
  std::unique_ptr<Shader> distanceShader;
};

SpriteBatch::SpriteBatch(DrawContext& context, int capacity) : impl_(std::make_unique<Impl>(context, capacity)){
}

SpriteBatch::~SpriteBatch(){
  TR_USE(CGE_SpriteBatch);
  if (!impl_->pending.empty())
    TR_WARN("%d sprites weren't flushed", (int)impl_->pending.size());
  //the context's pipelines refer to the shader
  if (impl_->shader)
    impl_->context.forgetShader(impl_->shader.get());
  if (impl_->distanceShader)
    impl_->context.forgetShader(impl_->distanceShader.get());
}

DrawContext& SpriteBatch::getContext() const{
  return impl_->context;
}

void SpriteBatch::setBlend(BlendMode mode){
  impl_->blend = mode;
}

BlendMode SpriteBatch::getBlend() const{
  return impl_->blend;
}

void SpriteBatch::setSampler(Sampler* sampler){
  impl_->sampler = sampler;
}

Sampler* SpriteBatch::getSampler() const{
  return impl_->sampler;
}

void SpriteBatch::setDistanceField(float edge){
  impl_->edge = edge < 0 ? -1 : edge;
}

float SpriteBatch::getDistanceField() const{
  return impl_->edge;
}

void SpriteBatch::draw(Texture* texture, const Matrix& transform, const TexRect& rect, const Color& color){
  //column-major: the images of the x- and y-axis and of the origin
  const float* m = transform.getData();
  Placement p;
  p.ax = m[0];
  p.ay = m[1];
  p.bx = m[4];
  p.by = m[5];
  p.ox = m[12];
  p.oy = m[13];
  p.oz = m[14];
  impl_->addPlacement(texture, p, rect, color);
}

void SpriteBatch::draw(Texture* texture, float x, float y, float width, float height, const TexRect& rect, const Color& color){
  Placement p = {width, 0, 0, height, x, y, 0};
  impl_->addPlacement(texture, p, rect, color);
}

void SpriteBatch::draw(Texture* texture, const Sprite& sprite, const TexRect& rect){
  TexRect r = rect;
  if (sprite.flipX)
    std::swap(r.u0, r.u1);
  if (sprite.flipY)
    std::swap(r.v0, r.v1);
  impl_->addPlacement(texture, place(sprite, 0, 0, 1, 1), r, sprite.color);
}

void SpriteBatch::draw(const AtlasRegion& region, const Sprite& sprite){
  if (region.width <= 0 || region.height <= 0)
    return;
  //the trimmed part inside the untrimmed image
  float s0 = region.trimX/region.width;
  float s1 = (region.trimX+region.trimWidth)/region.width;
  float t0 = region.trimY/region.height;
  float t1 = (region.trimY+region.trimHeight)/region.height;
  TexRect r = region.rect;
  if (sprite.flipX){
    float s = s0;
    s0 = 1-s1;
    s1 = 1-s;
    std::swap(r.u0, r.u1);
  }
  if (sprite.flipY){
    float t = t0;
    t0 = 1-t1;
    t1 = 1-t;
    std::swap(r.v0, r.v1);
  }
  impl_->addPlacement(region.texture, place(sprite, s0, t0, s1, t1), r, sprite.color);
}

void SpriteBatch::draw(const AtlasRegion& region, float x, float y, const Color& color){
  Sprite sprite = region.sprite(x, y);
  sprite.color = color;
  draw(region, sprite);
}

void SpriteBatch::flush(){
  impl_->flush();
}

int SpriteBatch::getPending() const{
  return (int)impl_->pending.size();
}

SpriteBatch::Stats SpriteBatch::getStats() const{
  if (impl_->frame != impl_->device.getFrameCount())
    return Stats();
  return impl_->stats;
}
