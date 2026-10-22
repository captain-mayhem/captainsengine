#include <gtest/gtest.h>

#include <cstring>
#include <vector>

#include <io/Tracing.h>
#include <io/TraceManager.h>
#include <renderer/renderdevice.h>
#include <renderer/shaderlibrary.h>

#include "testenv.h"
#include "testwindow.h"
#include "registerTestShaders.h"

using namespace CGE::Gfx;
using TestEnv::TestWindow;

// The shader library and the OpenGL backend's resources and draws. The GL
// tests draw into render targets on a hidden window and read the pixels
// back. They skip themselves where no window can be created or the driver
// has no OpenGL 3.3 core profile.

TEST(ShaderLibrary, ResolvesIncludes){
  std::string source;
  ASSERT_TRUE(ShaderLibrary::loadSource("engine/color.vert", source));
  EXPECT_NE(source.find("uniform CGE_Frame"), std::string::npos);
  EXPECT_NE(source.find("void main()"), std::string::npos);
  EXPECT_EQ(source.find("#include"), std::string::npos);
}

TEST(ShaderLibrary, IncludesBuiltinFilesFromAssets){
  std::string source;
  ASSERT_TRUE(ShaderLibrary::loadSource(TestEnv::dataFile("shaders/include.vert"), source));
  EXPECT_NE(source.find("uniform CGE_Draw"), std::string::npos);
  EXPECT_NE(source.find("a_test"), std::string::npos);
}

TEST(ShaderLibrary, MissingFilesFail){
  std::string source;
  EXPECT_FALSE(ShaderLibrary::loadSource("engine/missing.vert", source));
  EXPECT_FALSE(ShaderLibrary::loadSource(TestEnv::dataFile("shaders/missing.vert"), source));
  ShaderDesc desc;
  EXPECT_FALSE(ShaderLibrary::loadDesc("engine/color.vert", "engine/missing.frag", desc));
}

TEST(ShaderLibrary, FindsFilesCompiledIntoTheApplication){
  std::string source;
  //cge_embed_shaders in tests/CMakeLists.txt; adding them twice does no harm
  registerTestShaders();
  registerTestShaders();
  ASSERT_TRUE(ShaderLibrary::loadSource("test/embedded.vert", source));
  EXPECT_NE(source.find("a_embedded"), std::string::npos);
  EXPECT_NE(source.find("uniform CGE_Frame"), std::string::npos);
  EXPECT_FALSE(ShaderLibrary::loadSource("test/missing.vert", source));
}

TEST(ShaderLibrary, BuiltinDescriptionsBindTheEngineSlots){
  ShaderDesc desc = ShaderLibrary::getBuiltinDesc(BuiltinShader::Textured);
  EXPECT_EQ(desc.name, "engine/textured");
  EXPECT_FALSE(desc.vertexSource.empty());
  EXPECT_NE(desc.fragmentSource.find("cge_texture"), std::string::npos);
  ASSERT_EQ(desc.uniformBlocks.size(), 4u);
  EXPECT_EQ(desc.uniformBlocks[0].name, "CGE_Frame");
  EXPECT_EQ(desc.uniformBlocks[0].slot, SLOT_FRAME);
  EXPECT_EQ(desc.uniformBlocks[1].name, "CGE_Draw");
  EXPECT_EQ(desc.uniformBlocks[1].slot, SLOT_DRAW);
  EXPECT_EQ(desc.uniformBlocks[2].name, "CGE_Material");
  EXPECT_EQ(desc.uniformBlocks[2].slot, SLOT_MATERIAL);
  EXPECT_EQ(desc.uniformBlocks[3].name, "CGE_Lights");
  EXPECT_EQ(desc.uniformBlocks[3].slot, SLOT_LIGHTS);
  ASSERT_EQ(desc.textures.size(), 1u);
  EXPECT_EQ(desc.textures[0].slot, SLOT_TEXTURE);
}

#if defined(CGE_TEST_WINDOW)

namespace{

//! the vertex of the built-in shaders' pipelines in these tests
struct Vertex{
  float x, y, z;
  uint8_t color[4];
  float u, v;
};

VertexLayout vertexLayout(){
  VertexLayout layout;
  layout.attributes = {
    {ATTR_POSITION, VertexFormat::Float3, 0, 0},
    {ATTR_COLOR, VertexFormat::UByte4Norm, 0, 12},
    {ATTR_TEXCOORD, VertexFormat::Float2, 0, 16},
  };
  layout.buffers = {{(int)sizeof(Vertex), false}};
  return layout;
}

//! a white quad from (x0, y0) to (x1, y1) at depth z as a triangle strip
std::vector<Vertex> quad(float x0, float y0, float x1, float y1, float z = 0){
  return {
    {x0, y0, z, {255, 255, 255, 255}, 0, 0},
    {x1, y0, z, {255, 255, 255, 255}, 1, 0},
    {x0, y1, z, {255, 255, 255, 255}, 0, 1},
    {x1, y1, z, {255, 255, 255, 255}, 1, 1},
  };
}

struct Pixel{
  uint8_t r, g, b, a;
};

::testing::AssertionResult isNear(const Pixel& p, int r, int g, int b, int a = 255, int tolerance = 2){
  if (abs(p.r-r) <= tolerance && abs(p.g-g) <= tolerance && abs(p.b-b) <= tolerance && abs(p.a-a) <= tolerance)
    return ::testing::AssertionSuccess();
  return ::testing::AssertionFailure() << "pixel (" << (int)p.r << ", " << (int)p.g << ", " << (int)p.b << ", " << (int)p.a
    << ") instead of (" << r << ", " << g << ", " << b << ", " << a << ")";
}

class GLBackend : public ::testing::Test{
protected:
  void SetUp() override{
    device_ = RenderDevice::create("OpenGL");
    ASSERT_NE(device_, nullptr);
    DeviceDesc desc;
    desc.debug = true;
    if (!window_.create(*device_, desc))
      GTEST_SKIP() << "no window";
    if (!device_->init(window_.native(), desc))
      GTEST_SKIP() << "no OpenGL 3.3 core-profile driver";
    sampler_ = device_->createSampler(SamplerDesc());
    ASSERT_NE(sampler_, nullptr);
    FrameUniforms frame;
    frameBuffer_ = device_->createBuffer({BufferType::Uniform, Usage::Dynamic, sizeof(frame)}, &frame);
    ASSERT_NE(frameBuffer_, nullptr);
  }

  void TearDown() override{
    sampler_.reset();
    frameBuffer_.reset();
  }

  std::unique_ptr<RenderTarget> target(int width, int height, Format depth = Format::Undefined){
    RenderTargetDesc desc;
    desc.width = width;
    desc.height = height;
    desc.depthFormat = depth;
    return device_->createRenderTarget(desc);
  }

  std::unique_ptr<Buffer> vertices(const std::vector<Vertex>& data){
    return device_->createBuffer({BufferType::Vertex, Usage::Immutable, data.size()*sizeof(Vertex)}, data.data());
  }

  std::unique_ptr<Buffer> drawUniforms(float r, float g, float b, float a){
    DrawUniforms draw;
    draw.color[0] = r;
    draw.color[1] = g;
    draw.color[2] = b;
    draw.color[3] = a;
    return device_->createBuffer({BufferType::Uniform, Usage::Dynamic, sizeof(draw)}, &draw);
  }

  std::unique_ptr<Pipeline> pipeline(Shader* shader, PrimitiveType primitive = PrimitiveType::TriangleStrip){
    PipelineDesc desc;
    desc.shader = shader;
    desc.vertexLayout = vertexLayout();
    desc.primitive = primitive;
    return device_->createPipeline(desc);
  }

  //! starts a frame and a pass into target, cleared to the colour
  void begin(RenderTarget* target, float r, float g, float b){
    ASSERT_TRUE(device_->beginFrame());
    PassDesc pass;
    pass.target = target;
    pass.clearColor = {r, g, b, 1};
    ASSERT_TRUE(device_->beginPass(pass));
    device_->setUniformBuffer(SLOT_FRAME, frameBuffer_.get());
  }

  void end(){
    device_->endPass();
    device_->endFrame();
  }

  std::vector<Pixel> read(RenderTarget* target, int attachment = 0){
    const RenderTargetDesc& desc = target->getDesc();
    std::vector<Pixel> pixels(desc.width*desc.height);
    EXPECT_TRUE(device_->readPixels(target, attachment, {0, 0, desc.width, desc.height}, pixels.data()));
    return pixels;
  }

  //declared first, so the device goes before the window
  TestWindow window_;
  std::unique_ptr<RenderDevice> device_;
  std::unique_ptr<Sampler> sampler_;
  std::unique_ptr<Buffer> frameBuffer_;
};

}

TEST_F(GLBackend, ClearsRenderTargets){
  auto rt = target(4, 4);
  ASSERT_NE(rt, nullptr);
  begin(rt.get(), 1, 0, 0);
  end();
  for (const Pixel& p : read(rt.get()))
    EXPECT_TRUE(isNear(p, 255, 0, 0));
}

TEST_F(GLBackend, DrawsWithTheColorShader){
  auto rt = target(8, 4);
  auto shader = ShaderLibrary::createBuiltin(*device_, BuiltinShader::Color);
  ASSERT_NE(shader, nullptr);
  auto pipe = pipeline(shader.get());
  ASSERT_NE(pipe, nullptr);
  auto left = vertices(quad(-1, -1, 0, 1));
  auto green = drawUniforms(0, 1, 0, 1);
  begin(rt.get(), 0, 0, 0);
  device_->setPipeline(pipe.get());
  device_->setVertexBuffer(0, left.get());
  device_->setUniformBuffer(SLOT_DRAW, green.get());
  device_->draw(4);
  end();
  std::vector<Pixel> pixels = read(rt.get());
  for (int y = 0; y < 4; y++){
    EXPECT_TRUE(isNear(pixels[y*8+0], 0, 255, 0));
    EXPECT_TRUE(isNear(pixels[y*8+3], 0, 255, 0));
    EXPECT_TRUE(isNear(pixels[y*8+4], 0, 0, 0));
    EXPECT_TRUE(isNear(pixels[y*8+7], 0, 0, 0));
  }
}

TEST_F(GLBackend, BindsUniformBufferRanges){
  auto rt = target(8, 2);
  auto shader = ShaderLibrary::createBuiltin(*device_, BuiltinShader::Color);
  auto pipe = pipeline(shader.get());
  ASSERT_NE(pipe, nullptr);
  //two DrawUniforms in one buffer, at aligned offsets
  size_t align = device_->getCaps().uniformBufferOffsetAlignment;
  size_t stride = (sizeof(DrawUniforms)+align-1)/align*align;
  std::vector<uint8_t> data(stride+sizeof(DrawUniforms));
  DrawUniforms red, blue;
  red.color[1] = red.color[2] = 0;
  blue.color[0] = blue.color[1] = 0;
  memcpy(&data[0], &red, sizeof(red));
  memcpy(&data[stride], &blue, sizeof(blue));
  auto uniforms = device_->createBuffer({BufferType::Uniform, Usage::Immutable, data.size()}, data.data());
  std::vector<Vertex> all = quad(-1, -1, 0, 1);
  std::vector<Vertex> right = quad(0, -1, 1, 1);
  all.insert(all.end(), right.begin(), right.end());
  auto buffer = vertices(all);
  begin(rt.get(), 0, 0, 0);
  device_->setPipeline(pipe.get());
  device_->setVertexBuffer(0, buffer.get());
  device_->setUniformBuffer(SLOT_DRAW, uniforms.get(), 0, sizeof(DrawUniforms));
  device_->draw(4);
  device_->setUniformBuffer(SLOT_DRAW, uniforms.get(), stride, sizeof(DrawUniforms));
  device_->draw(4, 1, 4);
  end();
  std::vector<Pixel> pixels = read(rt.get());
  EXPECT_TRUE(isNear(pixels[1], 255, 0, 0));
  EXPECT_TRUE(isNear(pixels[6], 0, 0, 255));
}

TEST_F(GLBackend, SamplesTextures){
  auto rt = target(4, 4);
  auto shader = ShaderLibrary::createBuiltin(*device_, BuiltinShader::Textured);
  ASSERT_NE(shader, nullptr);
  auto pipe = pipeline(shader.get());
  //row 0 (the bottom): red, green; row 1: blue, white
  const uint8_t texels[] = {255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255, 255, 255, 255, 255};
  TextureDesc texDesc;
  texDesc.width = 2;
  texDesc.height = 2;
  auto texture = device_->createTexture(texDesc, texels);
  ASSERT_NE(texture, nullptr);
  SamplerDesc nearest;
  nearest.minFilter = nearest.magFilter = Filter::Nearest;
  auto sampler = device_->createSampler(nearest);
  auto full = vertices(quad(-1, -1, 1, 1));
  auto white = drawUniforms(1, 1, 1, 1);
  begin(rt.get(), 0, 0, 0);
  device_->setPipeline(pipe.get());
  device_->setVertexBuffer(0, full.get());
  device_->setUniformBuffer(SLOT_DRAW, white.get());
  device_->setTexture(SLOT_TEXTURE, texture.get(), sampler.get());
  device_->draw(4);
  end();
  std::vector<Pixel> pixels = read(rt.get());
  EXPECT_TRUE(isNear(pixels[0], 255, 0, 0));
  EXPECT_TRUE(isNear(pixels[3], 0, 255, 0));
  EXPECT_TRUE(isNear(pixels[12], 0, 0, 255));
  EXPECT_TRUE(isNear(pixels[15], 255, 255, 255));
}

TEST_F(GLBackend, UpdatesTexturesAndGeneratesMipmaps){
  auto shader = ShaderLibrary::createBuiltin(*device_, BuiltinShader::Textured);
  auto pipe = pipeline(shader.get());
  ASSERT_NE(pipe, nullptr);
  //4x4 black with the full mip chain, then a white checkerboard written into it
  TextureDesc texDesc;
  texDesc.width = 4;
  texDesc.height = 4;
  texDesc.mipLevels = 0;
  std::vector<uint8_t> black(4*4*4, 0);
  auto texture = device_->createTexture(texDesc, black.data());
  ASSERT_NE(texture, nullptr);
  std::vector<uint8_t> checker(4*4*4, 255);
  for (int i = 0; i < 16; i++){
    if ((i%4+i/4)%2)
      memset(&checker[i*4], 0, 3);
  }
  device_->updateTexture(texture.get(), {0, 0, 0, 0, 4, 4}, checker.data());
  device_->generateMipmaps(texture.get());
  SamplerDesc mip;
  mip.mipFilter = MipFilter::Linear;
  auto sampler = device_->createSampler(mip);
  auto full = vertices(quad(-1, -1, 1, 1));
  auto white = drawUniforms(1, 1, 1, 1);
  //drawn into one pixel, the 4x4 texture is sampled from its 1x1 level: grey
  auto rt = target(1, 1);
  begin(rt.get(), 1, 0, 0);
  device_->setPipeline(pipe.get());
  device_->setVertexBuffer(0, full.get());
  device_->setUniformBuffer(SLOT_DRAW, white.get());
  device_->setTexture(SLOT_TEXTURE, texture.get(), sampler.get());
  device_->draw(4);
  end();
  EXPECT_TRUE(isNear(read(rt.get())[0], 128, 128, 128, 255, 3));
}

TEST_F(GLBackend, Blends){
  auto rt = target(2, 2);
  auto shader = ShaderLibrary::createBuiltin(*device_, BuiltinShader::Color);
  PipelineDesc desc;
  desc.shader = shader.get();
  desc.vertexLayout = vertexLayout();
  desc.primitive = PrimitiveType::TriangleStrip;
  desc.blend.enabled = true;
  desc.blend.srcColor = BlendFactor::SrcAlpha;
  desc.blend.dstColor = BlendFactor::OneMinusSrcAlpha;
  auto pipe = device_->createPipeline(desc);
  ASSERT_NE(pipe, nullptr);
  auto full = vertices(quad(-1, -1, 1, 1));
  auto red = drawUniforms(1, 0, 0, 0.5f);
  begin(rt.get(), 0, 0, 1);
  device_->setPipeline(pipe.get());
  device_->setVertexBuffer(0, full.get());
  device_->setUniformBuffer(SLOT_DRAW, red.get());
  device_->draw(4);
  end();
  //alpha: 0.5*1 + 0*1
  EXPECT_TRUE(isNear(read(rt.get())[0], 128, 0, 128, 128));
}

TEST_F(GLBackend, TestsDepth){
  auto rt = target(2, 2, Format::Depth24);
  ASSERT_NE(rt, nullptr);
  auto shader = ShaderLibrary::createBuiltin(*device_, BuiltinShader::Color);
  PipelineDesc desc;
  desc.shader = shader.get();
  desc.vertexLayout = vertexLayout();
  desc.primitive = PrimitiveType::TriangleStrip;
  desc.depth.test = true;
  desc.depth.write = true;
  desc.depth.compare = CompareFunc::Less;
  auto pipe = device_->createPipeline(desc);
  std::vector<Vertex> data = quad(-1, -1, 1, 1, -0.5f);
  std::vector<Vertex> farQuad = quad(-1, -1, 1, 1, 0.5f);
  data.insert(data.end(), farQuad.begin(), farQuad.end());
  auto buffer = vertices(data);
  auto red = drawUniforms(1, 0, 0, 1);
  auto green = drawUniforms(0, 1, 0, 1);
  begin(rt.get(), 0, 0, 0);
  device_->setPipeline(pipe.get());
  device_->setVertexBuffer(0, buffer.get());
  device_->setUniformBuffer(SLOT_DRAW, red.get());
  device_->draw(4);
  device_->setUniformBuffer(SLOT_DRAW, green.get());
  device_->draw(4, 1, 4);
  end();
  EXPECT_TRUE(isNear(read(rt.get())[0], 255, 0, 0));
}

TEST_F(GLBackend, DrawsIndexedInstances){
  //the source has no #version line: the backend adds it
  ShaderDesc shaderDesc;
  shaderDesc.name = "instances";
  shaderDesc.vertexSource =
    "layout(location = 0) in vec2 a_position;\n"
    "layout(location = 4) in vec4 a_instance;\n"
    "layout(location = 5) in uvec4 a_id;\n"
    "out vec4 v_color;\n"
    "void main(){\n"
    "  v_color = vec4(a_instance.zw, float(a_id.x)/255.0, 1.0);\n"
    "  gl_Position = vec4(a_position+vec2(a_instance.x, 0.0), 0.0, 1.0);\n"
    "}\n";
  shaderDesc.fragmentSource =
    "in vec4 v_color;\n"
    "out vec4 o_color;\n"
    "void main(){\n"
    "  o_color = v_color;\n"
    "}\n";
  auto shader = device_->createShader(shaderDesc);
  ASSERT_NE(shader, nullptr);
  PipelineDesc desc;
  desc.shader = shader.get();
  desc.vertexLayout.attributes = {
    {0, VertexFormat::Float2, 0, 0},
    {4, VertexFormat::Float4, 1, 0},
    {5, VertexFormat::UByte4, 1, 16},
  };
  desc.vertexLayout.buffers = {{8, false}, {20, true}};
  auto pipe = device_->createPipeline(desc);
  ASSERT_NE(pipe, nullptr);
  const float corners[] = {-1, -1, 0, -1, 0, 1, -1, 1};
  auto vertexBuffer = device_->createBuffer({BufferType::Vertex, Usage::Immutable, sizeof(corners)}, corners);
  const uint16_t indices[] = {0, 1, 2, 0, 2, 3};
  auto indexBuffer = device_->createBuffer({BufferType::Index, Usage::Immutable, sizeof(indices)}, indices);
  //x offset, red, green, and an id in the blue channel
  struct Instance{
    float offset, unused, r, g;
    uint8_t id[4];
  };
  const Instance instances[] = {{0, 0, 1, 0, {0, 0, 0, 0}}, {1, 0, 0, 1, {255, 0, 0, 0}}};
  auto instanceBuffer = device_->createBuffer({BufferType::Vertex, Usage::Immutable, sizeof(instances)}, instances);
  auto rt = target(4, 2);
  begin(rt.get(), 0, 0, 0);
  device_->setPipeline(pipe.get());
  device_->setVertexBuffer(0, vertexBuffer.get());
  device_->setVertexBuffer(1, instanceBuffer.get());
  device_->setIndexBuffer(indexBuffer.get(), IndexType::UInt16);
  device_->drawIndexed(6, 2);
  end();
  std::vector<Pixel> pixels = read(rt.get());
  EXPECT_TRUE(isNear(pixels[0], 255, 0, 0));
  EXPECT_TRUE(isNear(pixels[3], 0, 255, 255));
}

TEST_F(GLBackend, DrawsIntoSeveralAttachments){
  ShaderDesc shaderDesc;
  shaderDesc.name = "two outputs";
  shaderDesc.vertexSource =
    "layout(location = 0) in vec3 a_position;\n"
    "void main(){\n"
    "  gl_Position = vec4(a_position, 1.0);\n"
    "}\n";
  shaderDesc.fragmentSource =
    "layout(location = 0) out vec4 o_first;\n"
    "layout(location = 1) out vec4 o_second;\n"
    "void main(){\n"
    "  o_first = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "  o_second = vec4(0.0, 1.0, 0.0, 1.0);\n"
    "}\n";
  auto shader = device_->createShader(shaderDesc);
  ASSERT_NE(shader, nullptr);
  auto pipe = pipeline(shader.get());
  RenderTargetDesc rtDesc;
  rtDesc.width = 2;
  rtDesc.height = 2;
  rtDesc.colorFormats = {Format::RGBA8, Format::RGBA8};
  auto rt = device_->createRenderTarget(rtDesc);
  ASSERT_NE(rt, nullptr);
  ASSERT_NE(rt->getColorTexture(1), nullptr);
  EXPECT_NE(rt->getDepthTexture(), nullptr);
  auto full = vertices(quad(-1, -1, 1, 1));
  begin(rt.get(), 0, 0, 0);
  device_->setPipeline(pipe.get());
  device_->setVertexBuffer(0, full.get());
  device_->draw(4);
  end();
  EXPECT_TRUE(isNear(read(rt.get(), 0)[0], 255, 0, 0));
  EXPECT_TRUE(isNear(read(rt.get(), 1)[0], 0, 255, 0));
}

TEST_F(GLBackend, UpdatesBuffers){
  auto rt = target(2, 2);
  auto shader = ShaderLibrary::createBuiltin(*device_, BuiltinShader::Color);
  auto pipe = pipeline(shader.get());
  auto full = vertices(quad(-1, -1, 1, 1));
  auto uniforms = drawUniforms(1, 0, 0, 1);
  DrawUniforms blue;
  blue.color[0] = blue.color[1] = 0;
  device_->updateBuffer(uniforms.get(), 0, sizeof(blue), &blue);
  begin(rt.get(), 0, 0, 0);
  device_->setPipeline(pipe.get());
  device_->setVertexBuffer(0, full.get());
  device_->setUniformBuffer(SLOT_DRAW, uniforms.get());
  device_->draw(4);
  end();
  EXPECT_TRUE(isNear(read(rt.get())[0], 0, 0, 255));
  //a part of it: green only
  const float green[] = {0, 1, 0, 1};
  device_->updateBuffer(uniforms.get(), offsetof(DrawUniforms, color), sizeof(green), green);
  begin(rt.get(), 0, 0, 0);
  device_->setPipeline(pipe.get());
  device_->setVertexBuffer(0, full.get());
  device_->setUniformBuffer(SLOT_DRAW, uniforms.get());
  device_->draw(4);
  end();
  EXPECT_TRUE(isNear(read(rt.get())[0], 0, 255, 0));
}

TEST_F(GLBackend, ReusedGLNamesDontFoolTheStateCache){
  auto rt = target(8, 2);
  auto shader = ShaderLibrary::createBuiltin(*device_, BuiltinShader::Color);
  auto pipe = pipeline(shader.get());
  auto white = drawUniforms(1, 1, 1, 1);
  auto left = vertices(quad(-1, -1, 0, 1));
  begin(rt.get(), 0, 0, 0);
  device_->setPipeline(pipe.get());
  device_->setUniformBuffer(SLOT_DRAW, white.get());
  device_->setVertexBuffer(0, left.get());
  device_->draw(4);
  //GL usually gives the new buffer the deleted one's name
  left.reset();
  auto right = vertices(quad(0, -1, 1, 1));
  device_->setVertexBuffer(0, right.get());
  device_->draw(4);
  end();
  std::vector<Pixel> pixels = read(rt.get());
  EXPECT_TRUE(isNear(pixels[1], 255, 255, 255));
  EXPECT_TRUE(isNear(pixels[6], 255, 255, 255));
}

TEST_F(GLBackend, CreatesTextureKinds){
  TextureDesc desc;
  desc.width = 8;
  desc.height = 8;
  desc.type = TextureType::Texture2DArray;
  desc.layers = 3;
  desc.format = Format::R8;
  std::vector<uint8_t> data(8*8*3, 7);
  EXPECT_NE(device_->createTexture(desc, data.data()), nullptr);
  desc.type = TextureType::Cube;
  desc.layers = 6;
  desc.format = Format::RGBA16F;
  EXPECT_NE(device_->createTexture(desc, nullptr), nullptr);
  desc.type = TextureType::Texture2D;
  desc.layers = 1;
  desc.format = Format::SRGBA8;
  desc.mipLevels = 0;
  EXPECT_NE(device_->createTexture(desc, nullptr), nullptr);
  desc.format = Format::Depth24Stencil8;
  desc.mipLevels = 1;
  EXPECT_NE(device_->createTexture(desc, nullptr), nullptr);
  //invalid ones
  desc.type = TextureType::Cube;
  desc.layers = 5;
  EXPECT_EQ(device_->createTexture(desc, nullptr), nullptr);
  desc.type = TextureType::Texture2D;
  desc.width = 0;
  EXPECT_EQ(device_->createTexture(desc, nullptr), nullptr);
}

TEST_F(GLBackend, RejectsBrokenShaders){
  ShaderDesc desc;
  desc.name = "broken";
  desc.vertexSource = "void main(){\n  gl_Position = undefined;\n}\n";
  desc.fragmentSource = "out vec4 o;\nvoid main(){ o = vec4(1.0); }\n";
  EXPECT_EQ(device_->createShader(desc), nullptr);
}

namespace{

//! collects the error messages that are traced
class ErrorCollector : public CGE::TraceOutputter{
public:
  bool init() override {return true;}
  void trace(unsigned channel, int level, const char* function, const char* message) override{
    if (level <= TRACE_ERROR)
      errors.push_back(message);
  }
  std::vector<std::string> errors;
};

}

TEST_F(GLBackend, TracesDriverErrors){
  //desc.debug is set, so KHR_debug reports GL errors into the tracing
  ErrorCollector collector;
  CGE::TraceManager::instance()->setTraceOutputter(&collector);
  //the manager replays what was traced before
  collector.errors.clear();
  auto rt = target(2, 2);
  begin(rt.get(), 0, 0, 0);
  //not checked by the device, but an error for GL
  device_->setViewport({0, 0, -1, -1});
  end();
  CGE::TraceManager::instance()->removeTraceOutputter(&collector);
  bool found = false;
  for (const std::string& error : collector.errors)
    found |= error.find("GL ") != std::string::npos;
  EXPECT_TRUE(found) << "no GL error traced";
}


#endif
