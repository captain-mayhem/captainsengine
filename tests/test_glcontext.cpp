#include <gtest/gtest.h>

#include <renderer/renderdevice.h>

#include "testwindow.h"

using namespace CGE::Gfx;
using TestEnv::TestWindow;

// The OpenGL backend's contexts on a real, hidden window. Skipped where no
// window can be created (Linux without a display) or the driver has no
// OpenGL 3.3 core profile (e.g. Windows' software renderer on CI machines).

#if defined(CGE_TEST_WINDOW)

namespace{

//! one frame with a cleared pass
void drawFrame(RenderDevice& device){
  ASSERT_TRUE(device.beginFrame());
  PassDesc pass;
  pass.colorLoad = LoadAction::Clear;
  pass.depthLoad = LoadAction::Clear;
  EXPECT_TRUE(device.beginPass(pass));
  device.endPass();
  device.endFrame();
}

}

TEST(GLContext, CoreProfile){
  //declared first, so the device goes before the window
  TestWindow window;
  auto device = RenderDevice::create("OpenGL");
  ASSERT_NE(device, nullptr);
  DeviceDesc desc;
  desc.debug = true;
  if (!window.create(*device, desc))
    GTEST_SKIP() << "no window";
  if (!device->init(window.native(), desc))
    GTEST_SKIP() << "no OpenGL 3.3 core-profile driver";
  //the minimums of OpenGL 3.3
  const Caps& caps = device->getCaps();
  EXPECT_GE(caps.maxTextureSize, 1024);
  EXPECT_GE(caps.maxTextureLayers, 256);
  EXPECT_GE(caps.maxColorAttachments, 8);
  EXPECT_GE(caps.maxVertexAttributes, 16);
  EXPECT_GE(caps.maxUniformBufferSize, 16384);
  EXPECT_GE(caps.uniformBufferOffsetAlignment, 1);
  EXPECT_GE(caps.maxAnisotropy, 1.0f);
  EXPECT_TRUE(caps.wireframe);
  drawFrame(*device);
}

TEST(GLContext, SecondContextOnAnotherWindow){
  TestWindow window, second;
  auto device = RenderDevice::create("OpenGL");
  ASSERT_NE(device, nullptr);
  DeviceDesc desc;
  if (!window.create(*device, desc))
    GTEST_SKIP() << "no window";
  if (!device->init(window.native(), desc))
    GTEST_SKIP() << "no OpenGL 3.3 core-profile driver";
  drawFrame(*device);
  device->setVsync(false);
  device->shutdown();
  //a second context on another window
  ASSERT_TRUE(second.create(*device, desc));
  ASSERT_TRUE(device->init(second.native(), desc));
  drawFrame(*device);
}

#endif
