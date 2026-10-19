#include <gtest/gtest.h>

#include <math/matrix.h>
#include <math/vector.h>
#include <renderer/nulldevice.h>

using namespace CGE;
using namespace CGE::Gfx;

// The render device interface, checked on the Null backend (no window
// needed). The OpenGL backend is only loaded here; drawing with it is checked
// in the applications.

namespace{

std::unique_ptr<NullDevice> makeDevice(){
  auto device = std::make_unique<NullDevice>();
  EXPECT_TRUE(device->init(NativeWindow(), DeviceDesc()));
  return device;
}

//! the Null device doesn't need a shader in its pipelines
std::unique_ptr<Pipeline> makePipeline(RenderDevice& device){
  PipelineDesc desc;
  desc.name = "test";
  return device.createPipeline(desc);
}

//! a device with Vulkan's clip space
class VulkanLikeDevice : public NullDevice{
protected:
  bool doInit(const NativeWindow& window, const DeviceDesc& desc) override{
    NullDevice::doInit(window, desc);
    clipSpace_.depthZeroToOne = true;
    clipSpace_.yDown = true;
    clipSpace_.originTopLeft = true;
    return true;
  }
};

}

TEST(RenderDevice, FactoryCreatesNullDevice){
  auto device = RenderDevice::create("Null");
  ASSERT_NE(device, nullptr);
  EXPECT_STREQ(device->getName(), "Null");
  EXPECT_FALSE(device->isInitialized());
  EXPECT_TRUE(device->init(NativeWindow(), DeviceDesc()));
  EXPECT_TRUE(device->isInitialized());
  device->shutdown();
  EXPECT_FALSE(device->isInitialized());
}

TEST(RenderDevice, FactoryRejectsUnknownBackend){
  EXPECT_EQ(RenderDevice::create("Glide"), nullptr);
}

#if !defined(ANDROID) && !defined(__EMSCRIPTEN__)
// the plugin library is found next to the engine library
TEST(RenderDevice, FactoryLoadsOpenGLPlugin){
  auto device = RenderDevice::create("OpenGL");
  ASSERT_NE(device, nullptr);
  EXPECT_STREQ(device->getName(), "OpenGL");
  EXPECT_FALSE(device->isInitialized());
  //a second device comes from the already loaded plugin
  auto second = RenderDevice::create("OpenGL");
  EXPECT_NE(second, nullptr);
}

TEST(RenderDevice, OpenGLNeedsAWindow){
  auto device = RenderDevice::create("OpenGL");
  ASSERT_NE(device, nullptr);
  EXPECT_FALSE(device->init(NativeWindow(), DeviceDesc()));
  EXPECT_FALSE(device->isInitialized());
}
#endif

TEST(RenderDevice, FramesNeedAnInitialisedDevice){
  NullDevice device;
  EXPECT_FALSE(device.beginFrame());
  device.endFrame();
  EXPECT_EQ(device.getFrameCount(), 0);
}

TEST(RenderDevice, FramesAndPasses){
  auto device = makeDevice();
  auto pipeline = makePipeline(*device);
  ASSERT_TRUE(device->beginFrame());
  //no nested frames, and no draws outside passes
  EXPECT_FALSE(device->beginFrame());
  device->setPipeline(pipeline.get());
  device->draw(3);
  EXPECT_EQ(device->getDrawCount(), 0);

  ASSERT_TRUE(device->beginPass(PassDesc()));
  EXPECT_TRUE(device->isInPass());
  EXPECT_FALSE(device->beginPass(PassDesc()));
  device->setPipeline(pipeline.get());
  device->draw(3);
  device->draw(3, 10);
  EXPECT_EQ(device->getDrawCount(), 2);
  device->endPass();
  EXPECT_FALSE(device->isInPass());
  device->endFrame();
  EXPECT_EQ(device->getFrameCount(), 1);
}

TEST(RenderDevice, PassesForgetThePipeline){
  auto device = makeDevice();
  auto pipeline = makePipeline(*device);
  device->beginFrame();
  device->beginPass(PassDesc());
  device->setPipeline(pipeline.get());
  device->endPass();
  device->beginPass(PassDesc());
  device->draw(3);
  EXPECT_EQ(device->getDrawCount(), 0);
  device->endPass();
  device->endFrame();
}

TEST(RenderDevice, EndFrameEndsAnOpenPass){
  auto device = makeDevice();
  device->beginFrame();
  device->beginPass(PassDesc());
  device->endFrame();
  EXPECT_FALSE(device->isInPass());
  EXPECT_EQ(device->getFrameCount(), 1);
  EXPECT_TRUE(device->beginFrame());
  device->endFrame();
}

TEST(RenderDevice, EmptyDrawsDontReachTheBackend){
  auto device = makeDevice();
  auto pipeline = makePipeline(*device);
  device->beginFrame();
  device->beginPass(PassDesc());
  device->setPipeline(pipeline.get());
  device->draw(0);
  device->draw(3, 0);
  EXPECT_EQ(device->getDrawCount(), 0);
  device->endPass();
  device->endFrame();
}

TEST(RenderDevice, IndexedDrawsNeedAnIndexBuffer){
  auto device = makeDevice();
  auto pipeline = makePipeline(*device);
  BufferDesc vertexDesc;
  vertexDesc.size = 64;
  auto vertices = device->createBuffer(vertexDesc, nullptr);
  BufferDesc indexDesc;
  indexDesc.type = BufferType::Index;
  indexDesc.size = 12;
  auto indices = device->createBuffer(indexDesc, nullptr);
  device->beginFrame();
  device->beginPass(PassDesc());
  device->setPipeline(pipeline.get());
  device->setVertexBuffer(0, vertices.get());
  device->drawIndexed(6);
  EXPECT_EQ(device->getDrawCount(), 0);
  //a vertex buffer isn't an index buffer
  device->setIndexBuffer(vertices.get(), IndexType::UInt16);
  device->drawIndexed(6);
  EXPECT_EQ(device->getDrawCount(), 0);
  device->setIndexBuffer(indices.get(), IndexType::UInt16);
  device->drawIndexed(6);
  EXPECT_EQ(device->getDrawCount(), 1);
  device->endPass();
  device->endFrame();
}

TEST(RenderDevice, ResourcesKeepTheirDescriptions){
  auto device = makeDevice();
  BufferDesc bufferDesc;
  bufferDesc.type = BufferType::Uniform;
  bufferDesc.usage = Usage::Dynamic;
  bufferDesc.size = 256;
  auto buffer = device->createBuffer(bufferDesc, nullptr);
  ASSERT_NE(buffer, nullptr);
  EXPECT_EQ(buffer->getDesc().type, BufferType::Uniform);
  EXPECT_EQ(buffer->getDesc().size, 256u);

  TextureDesc texDesc;
  texDesc.width = 64;
  texDesc.height = 32;
  texDesc.format = Format::R8;
  auto texture = device->createTexture(texDesc, nullptr);
  ASSERT_NE(texture, nullptr);
  EXPECT_EQ(texture->getDesc().width, 64);
  EXPECT_EQ(texture->getDesc().format, Format::R8);
}

TEST(RenderDevice, RenderTargetsOwnTheirAttachments){
  auto device = makeDevice();
  RenderTargetDesc desc;
  desc.width = 320;
  desc.height = 200;
  desc.colorFormats = {Format::RGBA8, Format::RGBA16F};
  desc.depthFormat = Format::Depth24Stencil8;
  auto target = device->createRenderTarget(desc);
  ASSERT_NE(target, nullptr);
  ASSERT_NE(target->getColorTexture(0), nullptr);
  ASSERT_NE(target->getColorTexture(1), nullptr);
  EXPECT_EQ(target->getColorTexture(2), nullptr);
  EXPECT_EQ(target->getColorTexture(1)->getDesc().format, Format::RGBA16F);
  EXPECT_EQ(target->getColorTexture(0)->getDesc().width, 320);
  ASSERT_NE(target->getDepthTexture(), nullptr);
  EXPECT_TRUE(isDepthFormat(target->getDepthTexture()->getDesc().format));

  desc.depthFormat = Format::Undefined;
  auto noDepth = device->createRenderTarget(desc);
  EXPECT_EQ(noDepth->getDepthTexture(), nullptr);
}

TEST(RenderDevice, UniformBufferOffsetsAreAligned){
  auto device = makeDevice();
  auto pipeline = makePipeline(*device);
  BufferDesc desc;
  desc.type = BufferType::Uniform;
  desc.size = 1024;
  auto buffer = device->createBuffer(desc, nullptr);
  ASSERT_EQ(device->getCaps().uniformBufferOffsetAlignment, 256);
  device->beginFrame();
  device->beginPass(PassDesc());
  //these are rejected with an error message; the device stays usable
  device->setUniformBuffer(0, buffer.get(), 100, 64);
  device->setUniformBuffer(0, buffer.get(), 1024, 64);
  device->setUniformBuffer(MAX_UNIFORM_BUFFERS, buffer.get());
  device->setUniformBuffer(0, buffer.get(), 256, 64);
  device->setPipeline(pipeline.get());
  device->draw(3);
  EXPECT_EQ(device->getDrawCount(), 1);
  device->endPass();
  device->endFrame();
}

TEST(RenderDevice, FormatSizes){
  EXPECT_EQ(formatSize(Format::R8), 1);
  EXPECT_EQ(formatSize(Format::RGBA8), 4);
  EXPECT_EQ(formatSize(Format::RGBA16F), 8);
  EXPECT_EQ(formatSize(Format::RGBA32F), 16);
  EXPECT_EQ(formatSize(Format::Undefined), 0);
  EXPECT_TRUE(isDepthFormat(Format::Depth16));
  EXPECT_FALSE(isDepthFormat(Format::R32F));
}

TEST(RenderDevice, OpenGLClipSpaceKeepsProjections){
  auto device = makeDevice();
  Matrix gl(Matrix::Ortho, -2, 2, -1, 1, 0.5f, 10);
  Matrix adjusted = device->adjustProjection(gl);
  for (int i = 0; i < 16; ++i)
    EXPECT_FLOAT_EQ(adjusted.at(i), gl.at(i));
}

TEST(RenderDevice, ProjectionsFollowTheClipSpace){
  VulkanLikeDevice device;
  ASSERT_TRUE(device.init(NativeWindow(), DeviceDesc()));
  Matrix proj = device.ortho(-2, 2, -1, 1, 1, 9);
  //the near plane maps to depth 0, the far plane to 1, and y points down
  Vec4f nearTop = proj*Vec4f(0, 1, -1, 1);
  Vec4f farBottom = proj*Vec4f(2, -1, -9, 1);
  EXPECT_NEAR(nearTop.z/nearTop.w, 0, 1e-5);
  EXPECT_NEAR(nearTop.y/nearTop.w, -1, 1e-5);
  EXPECT_NEAR(farBottom.z/farBottom.w, 1, 1e-5);
  EXPECT_NEAR(farBottom.y/farBottom.w, 1, 1e-5);
  EXPECT_NEAR(farBottom.x/farBottom.w, 1, 1e-5);

  Matrix persp = device.perspective(90, 2, 1, 100);
  Vec4f nearCorner = persp*Vec4f(2, 1, -1, 1);
  Vec4f farCentre = persp*Vec4f(0, 0, -100, 1);
  EXPECT_NEAR(nearCorner.x/nearCorner.w, 1, 1e-5);
  EXPECT_NEAR(nearCorner.y/nearCorner.w, -1, 1e-5);
  EXPECT_NEAR(nearCorner.z/nearCorner.w, 0, 1e-5);
  EXPECT_NEAR(farCentre.z/farCentre.w, 1, 1e-4);
}
