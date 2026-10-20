#include <gtest/gtest.h>

#include <renderer/renderdevice.h>

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(LINUX)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

using namespace CGE::Gfx;

// The OpenGL backend's contexts on a real, hidden window. Skipped where no
// window can be created (Linux without a display) and, for the core profile,
// where the driver has no OpenGL 3.3 (e.g. Windows' software renderer on CI
// machines). The compatibility profile falls back to the driver's default
// context, so it works with any OpenGL driver.

#if defined(WIN32) || defined(LINUX)

namespace{

//! a hidden native window for a device
class TestWindow{
public:
#if defined(WIN32)
  bool create(RenderDevice&, const DeviceDesc&){
    WNDCLASSA wc = {};
    //WGL keeps the device context of a CS_OWNDC window, like the engine's windows
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = DefWindowProcA;
    wc.hInstance = GetModuleHandleA(nullptr);
    wc.lpszClassName = "CGE_GLContextTest";
    RegisterClassA(&wc);
    wnd_ = CreateWindowA(wc.lpszClassName, "test", WS_OVERLAPPEDWINDOW, 0, 0, 64, 48, nullptr, nullptr, wc.hInstance, nullptr);
    return wnd_ != nullptr;
  }
  NativeWindow native() const{
    NativeWindow window;
    window.kind = NativeWindow::Win32;
    window.window = (uintptr_t)wnd_;
    return window;
  }
  ~TestWindow(){
    if (wnd_)
      DestroyWindow(wnd_);
  }
private:
  HWND wnd_ = nullptr;
#else
  bool create(RenderDevice& device, const DeviceDesc& desc){
    disp_ = XOpenDisplay(nullptr);
    if (!disp_)
      return false;
    int screen = DefaultScreen(disp_);
    XVisualInfo templ = {};
    templ.visualid = device.chooseX11Visual(disp_, screen, desc);
    int count = 0;
    XVisualInfo* visual = XGetVisualInfo(disp_, VisualIDMask, &templ, &count);
    if (!visual)
      return false;
    Window root = RootWindow(disp_, screen);
    XSetWindowAttributes attr = {};
    colormap_ = XCreateColormap(disp_, root, visual->visual, AllocNone);
    attr.colormap = colormap_;
    win_ = XCreateWindow(disp_, root, 0, 0, 64, 48, 0, visual->depth, InputOutput, visual->visual, CWColormap, &attr);
    XFree(visual);
    screen_ = screen;
    return win_ != 0;
  }
  NativeWindow native() const{
    NativeWindow window;
    window.kind = NativeWindow::X11;
    window.display = disp_;
    window.window = (uintptr_t)win_;
    window.screen = screen_;
    return window;
  }
  ~TestWindow(){
    if (win_)
      XDestroyWindow(disp_, win_);
    if (colormap_)
      XFreeColormap(disp_, colormap_);
    if (disp_)
      XCloseDisplay(disp_);
  }
private:
  Display* disp_ = nullptr;
  Window win_ = 0;
  Colormap colormap_ = 0;
  int screen_ = 0;
#endif
};

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

TEST(GLContext, CompatibilityProfile){
  TestWindow window, second;
  auto device = RenderDevice::create("OpenGL");
  ASSERT_NE(device, nullptr);
  DeviceDesc desc;
  desc.compatibilityProfile = true;
  if (!window.create(*device, desc))
    GTEST_SKIP() << "no window";
  ASSERT_TRUE(device->init(window.native(), desc));
  EXPECT_GE(device->getCaps().maxTextureSize, 64);
  drawFrame(*device);
  device->setVsync(false);
  device->shutdown();
  //a second context on another window
  ASSERT_TRUE(second.create(*device, desc));
  ASSERT_TRUE(device->init(second.native(), desc));
  drawFrame(*device);
}

#endif
