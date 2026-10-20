#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "GLContext.h"
#include "../../io/Tracing.h"

using namespace CGE::Gfx;

TR_CHANNEL_EXT(CGE_Renderer_GL);

// WGL_ARB_create_context and WGL_ARB_create_context_profile
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_FLAGS_ARB 0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_DEBUG_BIT_ARB 0x0001
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

namespace{

class WGLContext : public GLContext{
public:
  WGLContext() : wnd_(nullptr), dc_(nullptr), rc_(nullptr), opengl32_(nullptr) {}
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
    rc_ = createContext(desc, pixelFormat, pfd);
    if (!rc_ && desc.compatibilityProfile){
      //the old renderers also work on the driver's default context
      TR_WARN("No OpenGL 3.3 compatibility context, using the driver's default context");
      rc_ = wglCreateContext(dc_);
    }
    if (!rc_){
      TR_ERROR("Can't create an OpenGL 3.3 %s context", desc.compatibilityProfile ? "compatibility" : "core-profile");
      return false;
    }
    if (!wglMakeCurrent(dc_, rc_)){
      TR_ERROR("Can't activate the GL rendering context");
      return false;
    }
    opengl32_ = GetModuleHandleA("opengl32.dll");
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

  void* getProcAddress(const char* name) override{
    //wglGetProcAddress knows only the functions after OpenGL 1.1
    PROC proc = wglGetProcAddress(name);
    if (proc == nullptr || proc == (PROC)1 || proc == (PROC)2 || proc == (PROC)3 || proc == (PROC)-1)
      proc = opengl32_ ? GetProcAddress(opengl32_, name) : nullptr;
    return (void*)proc;
  }

private:
  //! the OpenGL 3.3 context, or nullptr if the driver can't create it
  /*! wglCreateContextAttribsARB is only available with a current context,
      so a helper context comes first. It lives on a hidden window of its own,
      so the window's DC only ever gets the final context (like GLFW and SDL). */
  HGLRC createContext(const DeviceDesc& desc, int pixelFormat, const PIXELFORMATDESCRIPTOR& pfd){
    TR_USE(CGE_Renderer_GL);
    WNDCLASSA wc = {};
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = DefWindowProcA;
    wc.hInstance = GetModuleHandleA(nullptr);
    wc.lpszClassName = "CGE_WGLHelper";
    //fails harmlessly when the class is already registered
    RegisterClassA(&wc);
    HWND helperWnd = CreateWindowA(wc.lpszClassName, "", WS_POPUP, 0, 0, 1, 1, nullptr, nullptr, wc.hInstance, nullptr);
    HDC helperDC = helperWnd ? GetDC(helperWnd) : nullptr;
    HGLRC helper = nullptr;
    if (helperDC && SetPixelFormat(helperDC, pixelFormat, &pfd))
      helper = wglCreateContext(helperDC);
    HGLRC rc = nullptr;
    if (helper && wglMakeCurrent(helperDC, helper)){
      typedef HGLRC (WINAPI *CreateContextAttribsFunc)(HDC dc, HGLRC share, const int* attribs);
      CreateContextAttribsFunc createContextAttribs = (CreateContextAttribsFunc)wglGetProcAddress("wglCreateContextAttribsARB");
      if (createContextAttribs){
        const int attribs[] = {
          WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
          WGL_CONTEXT_MINOR_VERSION_ARB, 3,
          WGL_CONTEXT_PROFILE_MASK_ARB, desc.compatibilityProfile ? WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB : WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
          WGL_CONTEXT_FLAGS_ARB, desc.debug ? WGL_CONTEXT_DEBUG_BIT_ARB : 0,
          0
        };
        rc = createContextAttribs(dc_, nullptr, attribs);
      }
      else
        TR_WARN("wglCreateContextAttribsARB not available");
      wglMakeCurrent(nullptr, nullptr);
    }
    else
      TR_WARN("Can't create the WGL helper context");
    if (helper)
      wglDeleteContext(helper);
    if (helperDC)
      ReleaseDC(helperWnd, helperDC);
    if (helperWnd)
      DestroyWindow(helperWnd);
    return rc;
  }

  HWND wnd_;
  HDC dc_;
  HGLRC rc_;
  HMODULE opengl32_;
};

}

std::unique_ptr<GLContext> GLContext::create(){
  return std::make_unique<WGLContext>();
}

#endif
