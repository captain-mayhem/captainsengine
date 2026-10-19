#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "GLContext.h"
#include "../../io/Tracing.h"

using namespace CGE::Gfx;

TR_CHANNEL_EXT(CGE_Renderer_GL);

namespace{

class WGLContext : public GLContext{
public:
  WGLContext() : wnd_(nullptr), dc_(nullptr), rc_(nullptr) {}
  ~WGLContext() {shutdown();}

  bool init(const NativeWindow& window, const DeviceDesc& desc, int& width, int& height) override{
    TR_USE(CGE_Renderer_GL);
    if (window.kind != NativeWindow::Win32 || !window.window){
      TR_ERROR("WGL needs a Win32 window");
      return false;
    }
    wnd_ = (HWND)window.window;
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = (BYTE)desc.colorBits;
    pfd.cDepthBits = (BYTE)desc.depthBits;
    pfd.cStencilBits = (BYTE)desc.stencilBits;
    pfd.iLayerType = PFD_MAIN_PLANE;

    //the window class has CS_OWNDC, so the DC stays valid
    if (!(dc_ = GetDC(wnd_))){
      TR_ERROR("Can't get the window's device context");
      return false;
    }
    int pixelFormat = ChoosePixelFormat(dc_, &pfd);
    if (!pixelFormat){
      TR_ERROR("Can't find a suitable pixel format");
      return false;
    }
    if (!SetPixelFormat(dc_, pixelFormat, &pfd)){
      TR_ERROR("Can't set the pixel format");
      return false;
    }
    if (!(rc_ = wglCreateContext(dc_))){
      TR_ERROR("Can't create the GL rendering context");
      return false;
    }
    if (!wglMakeCurrent(dc_, rc_)){
      TR_ERROR("Can't activate the GL rendering context");
      return false;
    }
    RECT rect;
    GetClientRect(wnd_, &rect);
    width = rect.right-rect.left;
    height = rect.bottom-rect.top;
    return true;
  }

  void shutdown() override{
    TR_USE(CGE_Renderer_GL);
    if (rc_){
      if (!wglMakeCurrent(nullptr, nullptr))
        TR_ERROR("Release of GL context failed");
      if (!wglDeleteContext(rc_))
        TR_ERROR("Release of rendering context failed");
      rc_ = nullptr;
    }
    if (dc_){
      if (!ReleaseDC(wnd_, dc_))
        TR_ERROR("Release of device context failed");
      dc_ = nullptr;
    }
  }

  void swapBuffers() override{
    SwapBuffers(dc_);
  }

  void setSwapInterval(int interval) override{
    TR_USE(CGE_Renderer_GL);
    typedef BOOL (WINAPI *SwapIntervalFunc)(int interval);
    SwapIntervalFunc swapInterval = (SwapIntervalFunc)wglGetProcAddress("wglSwapIntervalEXT");
    if (swapInterval)
      swapInterval(interval);
    else
      TR_WARN("wglSwapIntervalEXT not available, vsync can't be set");
  }

private:
  HWND wnd_;
  HDC dc_;
  HGLRC rc_;
};

}

std::unique_ptr<GLContext> GLContext::create(){
  return std::make_unique<WGLContext>();
}

#endif
