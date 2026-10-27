#include "Graphics.h"

#include <cstddef>

#include <image/image.h>
#include <io/Tracing.h>
#include <math/matrixstack.h>
#include <renderer/drawcontext.h>
#include <renderer/renderdevice.h>
#include <renderer/shaderlibrary.h>
#include <renderer/spritebatch.h>
#include <renderer/uniformstream.h>
#include <system/engine.h>

#include "Engine.h"
#include "registerAdventureShaders.h"

TR_CHANNEL(ADV_Graphics);

using namespace adv;
using namespace CGE::Gfx;

Graphics::Graphics() : mContext(*CGE::Engine::instance()->getDrawContext()), mDevice(mContext.getDevice()),
mInPass(false), mClearDepth(1.0f), mWindowMoving(false), mTexture(nullptr){
  //the shader files in shaders/ are compiled in (cge_embed_shaders)
  registerAdventureShaders();
  mTexTransform[0] = mTexTransform[1] = 1;
  mTexTransform[2] = mTexTransform[3] = 0;
  mTextured = createShader("blit.vert", "blit.frag");
  mUntextured = createShader("blit.vert", "blit_color.frag");
  Vertex quad[4];
  getQuad(quad);
  mQuad = createVertices(4, quad, Usage::Immutable);
  Vertex lines[5] = {quad[2], quad[3], quad[1], quad[0], quad[2]};
  mQuadLines = createVertices(5, lines, Usage::Immutable);
  SamplerDesc sampler;
  mSampler = mDevice.createSampler(sampler);
  mTextureSampler = mSampler.get();
  sampler.minFilter = sampler.magFilter = Filter::Nearest;
  sampler.addressU = sampler.addressV = AddressMode::ClampToEdge;
  mNearest = mDevice.createSampler(sampler);
  mBatch = std::make_unique<SpriteBatch>(mContext);
  mBatch->setSampler(mSampler.get());
}

Graphics::~Graphics(){
  mBatch.reset();
  destroyShader(mTextured);
  destroyShader(mUntextured);
}

RenderDevice& Graphics::getDevice(){
  return mDevice;
}

DrawContext& Graphics::getContext(){
  return mContext;
}

const VertexLayout& Graphics::getVertexLayout(){
  static VertexLayout layout;
  if (layout.attributes.empty()){
    VertexAttribute position;
    position.location = ATTR_POSITION;
    position.format = VertexFormat::Float3;
    position.offset = offsetof(Vertex, x);
    VertexAttribute texcoord;
    texcoord.location = ATTR_TEXCOORD;
    texcoord.format = VertexFormat::Float2;
    texcoord.offset = offsetof(Vertex, u);
    layout.attributes = {position, texcoord};
    VertexBufferLayout buffer;
    buffer.stride = sizeof(Vertex);
    layout.buffers = {buffer};
  }
  return layout;
}

void Graphics::getQuad(Vertex quad[4]){
  quad[0] = {0, 1, 0, 0, 1};
  quad[1] = {0, 0, 0, 0, 0};
  quad[2] = {1, 1, 0, 1, 1};
  quad[3] = {1, 0, 0, 1, 0};
}

bool Graphics::beginFrame(){
  TR_USE(ADV_Graphics);
  if (!mTargets.empty()){
    TR_ERROR("beginFrame: the last frame wasn't ended");
    mTargets.clear();
  }
  mContext.model().loadIdentity();
  mContext.setView(CGE::Matrix(CGE::Matrix::Identity));
  //the frame is drawn into a render target of the window's size, which keeps
  //its contents from frame to frame (a window's contents may be lost);
  //the game clears it (clear), except where it doesn't while the camera's
  //viewport moves
  Vec2i size(mDevice.getWidth(), mDevice.getHeight());
  bool keep = true;
  if (!mScreen || size.x != mScreenSize.x || size.y != mScreenSize.y){
    mScreen.reset();
    if (size.x <= 0 || size.y <= 0)
      return false;
    RenderTargetDesc desc;
    desc.width = size.x;
    desc.height = size.y;
    desc.depthFormat = Format::Undefined;
    mScreen = mDevice.createRenderTarget(desc);
    mScreenSize = size;
    keep = false;
    if (!mScreen){
      TR_ERROR("Cannot create the screen's render target");
      return false;
    }
  }
  mTargets.push_back({mScreen.get(), mScreenSize, true});
  return beginPass(keep ? LoadAction::Load : LoadAction::Clear);
}

void Graphics::endFrame(){
  TR_USE(ADV_Graphics);
  if (mTargets.size() != 1)
    TR_ERROR("endFrame: %i render targets weren't popped", (int)mTargets.size()-1);
  flushSprites();
  if (mInPass)
    mDevice.endPass();
  mInPass = false;
  mTargets.clear();
  if (!mScreen)
    return;

  //the frame into the window, pixel by pixel
  PassDesc pass;
  pass.colorLoad = LoadAction::DontCare;
  if (!mDevice.beginPass(pass))
    return;
  mInPass = true;
  CGE::Gfx::BlendMode blend = mContext.getBlend();
  CGE::Color color = mContext.getColor();
  mContext.setBlend(BlendMode::Opaque);
  mContext.setColor(1.0f, 1.0f, 1.0f, 1.0f);
  mContext.setProjection(mDevice.ortho(0.0f, (float)mScreenSize.x, 0.0f, (float)mScreenSize.y, -1.0f, 1.0f));
  mContext.model().push();
  mContext.model().loadIdentity();
  mContext.model().scale((float)mScreenSize.x, (float)mScreenSize.y, 1.0f);
  setTexture(mScreen->getColorTexture(0));
  mTextureSampler = mNearest.get();
  drawQuad();
  setTexture(nullptr);
  //the context mustn't keep it: the screen is made anew when the window's size changes
  mContext.setTexture(nullptr);
  mContext.model().pop();
  mContext.setColor(color);
  mContext.setBlend(blend);
  mDevice.endPass();
  mInPass = false;
}

void Graphics::pushTarget(RenderTarget* target, const Vec2i& size){
  flushSprites();
  if (mInPass)
    mDevice.endPass();
  mInPass = false;
  mTargets.push_back({target, size, false});
  beginPass(LoadAction::Load);
}

void Graphics::popTarget(){
  TR_USE(ADV_Graphics);
  if (mTargets.empty() || mTargets.back().screen){
    TR_ERROR("popTarget without pushTarget");
    return;
  }
  flushSprites();
  if (mInPass)
    mDevice.endPass();
  mInPass = false;
  mTargets.pop_back();
  if (!mTargets.empty())
    beginPass(LoadAction::Load);
}

void Graphics::setWindowOffset(const Vec2f& offset, bool moving){
  //the waiting sprites keep the projection they were drawn with
  flushSprites();
  mWindowOffset = offset;
  mWindowMoving = moving;
  if (mInPass && mTargets.back().screen)
    setupTarget();
}

void Graphics::setClearColor(float r, float g, float b, float a){
  mClearColor = {r, g, b, a};
}

void Graphics::setClearDepth(float depth){
  mClearDepth = depth;
}

void Graphics::clear(){
  TR_USE(ADV_Graphics);
  if (mTargets.empty()){
    TR_ERROR("clear outside of a frame");
    return;
  }
  flushSprites();
  if (mTargets.back().screen && mWindowMoving){
    //only the area the game moved to, as with a scissor rectangle
    CGE::Gfx::BlendMode blend = mContext.getBlend();
    mContext.setBlend(BlendMode::Opaque);
    CGE::Color color = mContext.getColor();
    Texture* texture = mTexture;
    mTexture = nullptr;
    mContext.setColor(mClearColor.r, mClearColor.g, mClearColor.b, mClearColor.a);
    Vec2i res = Engine::instance()->getResolution();
    mContext.model().push();
    mContext.model().loadIdentity();
    mContext.model().scale((float)res.x, (float)res.y, 1.0f);
    drawQuad();
    mContext.model().pop();
    mTexture = texture;
    mContext.setColor(color);
    mContext.setBlend(blend);
    return;
  }
  if (mInPass)
    mDevice.endPass();
  mInPass = false;
  beginPass(LoadAction::Clear);
}

bool Graphics::beginPass(LoadAction load){
  TR_USE(ADV_Graphics);
  PassDesc pass;
  pass.target = mTargets.back().target;
  pass.colorLoad = load;
  pass.depthLoad = load;
  pass.clearColor = mClearColor;
  pass.clearDepth = mClearDepth;
  mInPass = mDevice.beginPass(pass);
  if (!mInPass){
    TR_ERROR("Cannot begin a pass");
    return false;
  }
  setupTarget();
  return true;
}

void Graphics::setupTarget(){
  const Target& top = mTargets.back();
  if (top.screen){
    //the game in the window as the virtual screen places it, y down
    CGE::VirtualScreen& screen = CGE::Engine::instance()->getVirtualScreen();
    CGE::Vec2f vmin = screen.getVisibleMin();
    CGE::Vec2f vmax = screen.getVisibleMax();
    CGE::Matrix projection = mDevice.ortho(vmin.x, vmax.x, vmax.y, vmin.y, -1.0f, 1.0f);
    if (mWindowOffset.x != 0 || mWindowOffset.y != 0)
      projection = projection*CGE::Matrix(CGE::Matrix::Translation, CGE::Vec3f(mWindowOffset.x, -mWindowOffset.y, 0.0f));
    mContext.setProjection(projection);
  }
  else{
    //the resolution onto the used part of the target
    Viewport viewport;
    viewport.width = (float)top.size.x;
    viewport.height = (float)top.size.y;
    mDevice.setViewport(viewport);
    Vec2i res = Engine::instance()->getResolution();
    mContext.setProjection(mDevice.ortho(0.0f, (float)res.x, (float)res.y, 0.0f, -1.0f, 1.0f));
  }
}

CGE::MatrixStack& Graphics::model(){
  return mContext.model();
}

void Graphics::setColor(float r, float g, float b, float a){
  mContext.setColor(r, g, b, a);
}

void Graphics::setBlend(Blend blend){
  switch (blend){
    case BLEND_ALPHA:
      mContext.setBlend(BlendMode::Alpha);
      break;
    case BLEND_ADDITIVE:
      mContext.setBlend(BlendMode::Additive);
      break;
    case BLEND_PREMULT_ALPHA:
      mContext.setBlend(BlendMode::PremultipliedAlpha);
      break;
    case BLEND_MULTIPLY:
      mContext.setBlend(BlendMode::Multiply);
      break;
  }
}

void Graphics::setTexture(Texture* texture, const Vec2f& scale, const Vec2f& offset){
  mTexture = texture;
  mTextureSampler = mSampler.get();
  mTexTransform[0] = scale.x;
  mTexTransform[1] = scale.y;
  mTexTransform[2] = offset.x;
  mTexTransform[3] = offset.y;
}

void Graphics::setDepth(bool test, bool write, CompareFunc compare){
  flushSprites();
  mContext.setDepth(test, write, compare);
}

void Graphics::setColorWrite(bool write){
  flushSprites();
  mContext.setColorWrite(write);
}

Sampler* Graphics::getSampler(){
  return mSampler.get();
}

Buffer* Graphics::getQuadBuffer(){
  return mQuad.get();
}

bool Graphics::prepareDraw(PrimitiveType primitive){
  TR_USE(ADV_Graphics);
  if (!mInPass){
    TR_ERROR("Drawing outside of a pass");
    return false;
  }
  return prepare(mTexture ? mTextured.get() : mUntextured.get(), getVertexLayout(), primitive, mTexTransform, sizeof(mTexTransform));
}

void Graphics::drawSprite(Texture* texture, const Vec2f& scale, const Vec2f& offset, float r, float g, float b, float a){
  TR_USE(ADV_Graphics);
  if (!mInPass){
    TR_ERROR("Drawing outside of a pass");
    return;
  }
  TexRect rect;
  rect.u0 = offset.x;
  rect.v0 = offset.y;
  rect.u1 = offset.x+scale.x;
  rect.v1 = offset.y+scale.y;
  mBatch->setBlend(mContext.getBlend());
  mBatch->draw(texture, mContext.model().top(), rect, CGE::Color(r, g, b, a));
}

void Graphics::flushSprites(){
  if (mInPass)
    mBatch->flush();
}

void Graphics::drawQuad(){
  draw(mQuad.get(), PrimitiveType::TriangleStrip, 4);
}

void Graphics::drawQuadLines(){
  draw(mQuadLines.get(), PrimitiveType::LineStrip, 5);
}

void Graphics::draw(Buffer* vertices, PrimitiveType primitive, int count, int first){
  if (!vertices || count <= 0 || !prepareDraw(primitive))
    return;
  mDevice.setVertexBuffer(0, vertices);
  mDevice.draw(count, 1, first);
}

bool Graphics::prepare(Shader* shader, const VertexLayout& layout, PrimitiveType primitive, const void* uniforms, size_t size){
  //the sprites before come first
  flushSprites();
  //the context keeps the texture it had, which may be deleted by now
  mContext.setTexture(mTexture, mTextureSampler);
  if (!mContext.prepare(shader, layout, primitive))
    return false;
  if (uniforms && !mContext.getUniformStream().bind(SLOT_ADVENTURE, uniforms, size))
    return false;
  return true;
}

std::unique_ptr<Buffer> Graphics::createVertices(int count, const Vertex* vertices, Usage usage){
  BufferDesc desc;
  desc.type = BufferType::Vertex;
  desc.usage = usage;
  desc.size = count*sizeof(Vertex);
  return mDevice.createBuffer(desc, vertices);
}

std::unique_ptr<Shader> Graphics::createShader(const char* vertexFile, const char* fragmentFile, const std::vector<const char*>& samplers){
  TR_USE(ADV_Graphics);
  ShaderDesc desc;
  std::string vertex = std::string("adventure/")+vertexFile;
  std::string fragment = std::string("adventure/")+fragmentFile;
  desc.name = vertex+" "+fragment;
  if (!ShaderLibrary::loadDesc(vertex, fragment, desc)){
    TR_ERROR("Shader %s not found", desc.name.c_str());
    return nullptr;
  }
  desc.uniformBlocks.push_back({"ADV_Blit", SLOT_ADVENTURE});
  desc.uniformBlocks.push_back({"ADV_Effect", SLOT_ADVENTURE});
  desc.textures.push_back({"tex", SLOT_TEXTURE});
  int slot = SLOT_TEXTURE+1;
  for (const char* sampler : samplers)
    desc.textures.push_back({sampler, slot++});
  return mDevice.createShader(desc);
}

void Graphics::destroyShader(std::unique_ptr<Shader>& shader){
  //the context's pipelines refer to it
  if (shader)
    mContext.forgetShader(shader.get());
  shader.reset();
}

Texture* Graphics::createTexture(int width, int height, const CGE::Image* image, const CGE::Image* alpha){
  TextureDesc desc;
  desc.format = Format::RGBA8;
  desc.width = width;
  desc.height = height;
  Texture* texture = mDevice.createTexture(desc).release();
  if (texture && image)
    updateTexture(texture, image, alpha);
  return texture;
}

void Graphics::updateTexture(Texture* texture, const CGE::Image* image, const CGE::Image* alpha){
  if (!texture || !image)
    return;
  std::vector<unsigned char> pixels = toRGBA(image, alpha);
  TextureRegion region;
  region.width = (int)image->getWidth();
  region.height = (int)image->getHeight();
  mDevice.updateTexture(texture, region, pixels.data());
}

std::vector<unsigned char> Graphics::toRGBA(const CGE::Image* image, const CGE::Image* alpha){
  unsigned width = image->getWidth();
  unsigned count = width*image->getHeight();
  std::vector<unsigned char> buffer(count*4);
  const unsigned char* data = image->getData();
  unsigned channels = image->getNumChannels();
  for (unsigned i = 0; i < count; ++i){
    if (channels < 3){
      buffer[4*i] = buffer[4*i+1] = buffer[4*i+2] = data[channels*i];
      buffer[4*i+3] = channels == 2 ? data[channels*i+1] : 255;
      continue;
    }
    buffer[4*i] = data[channels*i];
    buffer[4*i+1] = data[channels*i+1];
    buffer[4*i+2] = data[channels*i+2];
    if (image->hasAlpha())
      buffer[4*i+3] = data[channels*i+3];
    else if (alpha)
      buffer[4*i+3] = alpha->getPixelChannel(i%width, i/width, 0);
    else if (data[channels*i] == 0xFF && data[channels*i+1] == 0x0 && data[channels*i+2] == 0xFF)
      buffer[4*i+3] = 0;
    else
      buffer[4*i+3] = 255;
  }
  return buffer;
}
