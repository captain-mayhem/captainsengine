#if defined(LINUX)
#include <cstring>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "GLContext.h"
#include "../../io/Tracing.h"

using namespace CGE::Gfx;

TR_CHANNEL_EXT(CGE_Renderer_GL);

namespace{

//! a desktop OpenGL context through EGL on an X11 window
/*! Replaces GLX; the same code works on Wayland later with another native
    display and window. */
class EGLX11Context : public GLContext{
public:
  EGLX11Context() : display_(EGL_NO_DISPLAY), config_(nullptr), surface_(EGL_NO_SURFACE), ctx_(EGL_NO_CONTEXT) {}
  ~EGLX11Context() {shutdown();}

  unsigned long chooseX11Visual(void* display, int screen, const DeviceDesc& desc) override{
    TR_USE(CGE_Renderer_GL);
    if (!initDisplay((Display*)display))
      return 0;
    EGLConfig config;
    EGLint count = 0;
    if (!chooseConfigs(desc, &config, 1, count) || count == 0){
      TR_ERROR("No suitable EGL config");
      return 0;
    }
    config_ = config;
    EGLint visual = 0;
    eglGetConfigAttrib(display_, config_, EGL_NATIVE_VISUAL_ID, &visual);
    return (unsigned long)visual;
  }

  bool init(const NativeWindow& window, const DeviceDesc& desc, int& width, int& height) override{
    TR_USE(CGE_Renderer_GL);
    if (window.kind != NativeWindow::X11 || !window.display || !window.window){
      TR_ERROR("EGL on X11 needs an X11 window");
      return false;
    }
    Display* disp = (Display*)window.display;
    Window win = (Window)window.window;
    if (!initDisplay(disp))
      return false;
    XWindowAttributes attr;
    XGetWindowAttributes(disp, win, &attr);
    if (!chooseWindowConfig(desc, XVisualIDFromVisual(attr.visual)))
      return false;

    surface_ = eglCreateWindowSurface(display_, config_, (EGLNativeWindowType)win, nullptr);
    if (surface_ == EGL_NO_SURFACE){
      TR_ERROR("Can't create the EGL window surface (error 0x%x)", eglGetError());
      return false;
    }
    if (!eglBindAPI(EGL_OPENGL_API)){
      TR_ERROR("EGL can't create desktop OpenGL contexts");
      return false;
    }
    ctx_ = createContext(desc);
    if (ctx_ == EGL_NO_CONTEXT){
      TR_ERROR("Can't create an OpenGL 3.3 core-profile context (error 0x%x)", eglGetError());
      return false;
    }
    if (!eglMakeCurrent(display_, surface_, surface_, ctx_)){
      TR_ERROR("Can't activate the EGL context (error 0x%x)", eglGetError());
      return false;
    }
    width = attr.width;
    height = attr.height;
    return true;
  }

  void shutdown() override{
    if (display_ == EGL_NO_DISPLAY)
      return;
    eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (ctx_ != EGL_NO_CONTEXT)
      eglDestroyContext(display_, ctx_);
    if (surface_ != EGL_NO_SURFACE)
      eglDestroySurface(display_, surface_);
    eglTerminate(display_);
    eglReleaseThread();
    display_ = EGL_NO_DISPLAY;
    config_ = nullptr;
    surface_ = EGL_NO_SURFACE;
    ctx_ = EGL_NO_CONTEXT;
  }

  void swapBuffers() override{
    eglSwapBuffers(display_, surface_);
  }

  void setSwapInterval(int interval) override{
    TR_USE(CGE_Renderer_GL);
    if (!eglSwapInterval(display_, interval))
      TR_WARN("eglSwapInterval failed, vsync can't be set");
  }

  void* getProcAddress(const char* name) override{
    //EGL 1.5 (and EGL_KHR_get_all_proc_addresses) also returns the OpenGL 1.x functions
    return (void*)eglGetProcAddress(name);
  }

private:
  bool initDisplay(Display* disp){
    TR_USE(CGE_Renderer_GL);
    if (display_ != EGL_NO_DISPLAY)
      return true;
    //the X11 platform explicitly, so EGL doesn't have to guess what the display is
    const char* clientExtensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    PFNEGLGETPLATFORMDISPLAYEXTPROC getPlatformDisplay = (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");
    if (clientExtensions && strstr(clientExtensions, "EGL_EXT_platform_x11") && getPlatformDisplay)
      display_ = getPlatformDisplay(EGL_PLATFORM_X11_EXT, disp, nullptr);
    else
      display_ = eglGetDisplay((EGLNativeDisplayType)disp);
    EGLint major = 0, minor = 0;
    if (display_ == EGL_NO_DISPLAY || !eglInitialize(display_, &major, &minor)){
      TR_ERROR("Can't initialise EGL on the X11 display");
      display_ = EGL_NO_DISPLAY;
      return false;
    }
    TR_INFO("EGL %d.%d (%s)", major, minor, eglQueryString(display_, EGL_VENDOR));
    return true;
  }

  bool chooseConfigs(const DeviceDesc& desc, EGLConfig* configs, EGLint size, EGLint& count){
    const EGLint attribs[] = {
      EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
      EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
      EGL_RED_SIZE, 8,
      EGL_GREEN_SIZE, 8,
      EGL_BLUE_SIZE, 8,
      EGL_DEPTH_SIZE, desc.depthBits,
      EGL_STENCIL_SIZE, desc.stencilBits,
      EGL_NONE
    };
    return eglChooseConfig(display_, attribs, configs, size, &count) && count > 0;
  }

  //! the config of the window's visual (normally the one chooseX11Visual picked)
  bool chooseWindowConfig(const DeviceDesc& desc, VisualID visual){
    TR_USE(CGE_Renderer_GL);
    EGLConfig configs[64];
    EGLint count = 0;
    if (!chooseConfigs(desc, configs, 64, count)){
      TR_ERROR("No suitable EGL config");
      return false;
    }
    for (EGLint i = 0; i < count; ++i){
      EGLint id = 0;
      eglGetConfigAttrib(display_, configs[i], EGL_NATIVE_VISUAL_ID, &id);
      if ((VisualID)id == visual){
        config_ = configs[i];
        return true;
      }
    }
    TR_WARN("No EGL config has the window's visual, using another one");
    config_ = configs[0];
    return true;
  }

  EGLContext createContext(const DeviceDesc& desc){
    const EGLint attribs[] = {
      EGL_CONTEXT_MAJOR_VERSION_KHR, 3,
      EGL_CONTEXT_MINOR_VERSION_KHR, 3,
      EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR,
      EGL_CONTEXT_FLAGS_KHR, desc.debug ? EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR : 0,
      EGL_NONE
    };
    return eglCreateContext(display_, config_, EGL_NO_CONTEXT, attribs);
  }

  EGLDisplay display_;
  EGLConfig config_;
  EGLSurface surface_;
  EGLContext ctx_;
};

}

std::unique_ptr<GLContext> GLContext::create(){
  return std::make_unique<EGLX11Context>();
}

#endif
