#if defined(ANDROID)
#include <cstring>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <android/native_window.h>

#include "GLContext.h"
#include "../../io/Tracing.h"

using namespace CGE::Gfx;

TR_CHANNEL_EXT(CGE_Renderer_GL);

namespace{

//! an OpenGL ES 3.0 context on the activity's window
/*! The window goes away whenever the application goes into the background.
    The context outlives it: it stays current on a 1x1 pbuffer, or on no
    surface at all with EGL_KHR_surfaceless_context, until the next window
    comes, so the resources survive. Only a lost context (EGL_CONTEXT_LOST,
    after power management events on some devices) loses them. */
class EGLContextImpl : public GLContext{
public:
  EGLContextImpl() : display_(EGL_NO_DISPLAY), config_(nullptr), surface_(EGL_NO_SURFACE), idle_(EGL_NO_SURFACE),
    ctx_(EGL_NO_CONTEXT), surfaceless_(false), lost_(false) {}
  ~EGLContextImpl() {shutdown();}

  bool init(const NativeWindow& window, const DeviceDesc& desc, int& width, int& height) override{
    TR_USE(CGE_Renderer_GL);
    if (window.kind != NativeWindow::Android || !window.window){
      TR_ERROR("EGL needs an Android window");
      return false;
    }
    display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display_ == EGL_NO_DISPLAY || !eglInitialize(display_, nullptr, nullptr)){
      TR_ERROR("Can't initialise EGL");
      return false;
    }
    const char* extensions = eglQueryString(display_, EGL_EXTENSIONS);
    surfaceless_ = extensions && strstr(extensions, "EGL_KHR_surfaceless_context") != nullptr;
    //8 bits per channel; the window has no alpha
    const EGLint attribs[] = {
      EGL_SURFACE_TYPE, surfaceless_ ? EGL_WINDOW_BIT : (EGL_WINDOW_BIT | EGL_PBUFFER_BIT),
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
      EGL_RED_SIZE, 8,
      EGL_GREEN_SIZE, 8,
      EGL_BLUE_SIZE, 8,
      EGL_DEPTH_SIZE, desc.depthBits,
      EGL_STENCIL_SIZE, desc.stencilBits,
      EGL_NONE
    };
    EGLint numConfigs = 0;
    if (!eglChooseConfig(display_, attribs, &config_, 1, &numConfigs) || numConfigs == 0){
      TR_ERROR("No EGL config for OpenGL ES 3.0");
      return false;
    }
    const EGLint ctxAttribs[] = {
      EGL_CONTEXT_CLIENT_VERSION, 3,
      EGL_NONE
    };
    ctx_ = eglCreateContext(display_, config_, EGL_NO_CONTEXT, ctxAttribs);
    if (ctx_ == EGL_NO_CONTEXT){
      TR_ERROR("Can't create an OpenGL ES 3.0 context (0x%x)", eglGetError());
      return false;
    }
    if (!surfaceless_){
      const EGLint pbufferAttribs[] = {EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE};
      idle_ = eglCreatePbufferSurface(display_, config_, pbufferAttribs);
      if (idle_ == EGL_NO_SURFACE){
        TR_ERROR("Can't create the EGL pbuffer (0x%x)", eglGetError());
        return false;
      }
    }
    lost_ = false;
    if (!attachWindow(window, width, height))
      return false;
    TR_INFO("Made the EGL context current%s", surfaceless_ ? "" : " (with a pbuffer in the background)");
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
    if (idle_ != EGL_NO_SURFACE)
      eglDestroySurface(display_, idle_);
    eglTerminate(display_);
    display_ = EGL_NO_DISPLAY;
    config_ = nullptr;
    surface_ = EGL_NO_SURFACE;
    idle_ = EGL_NO_SURFACE;
    ctx_ = EGL_NO_CONTEXT;
  }

  void releaseWindow() override{
    if (display_ == EGL_NO_DISPLAY || surface_ == EGL_NO_SURFACE)
      return;
    //the context stays current, without the window's surface
    if (!eglMakeCurrent(display_, idle_, idle_, ctx_))
      checkLost("eglMakeCurrent");
    eglDestroySurface(display_, surface_);
    surface_ = EGL_NO_SURFACE;
  }

  bool attachWindow(const NativeWindow& window, int& width, int& height) override{
    TR_USE(CGE_Renderer_GL);
    if (display_ == EGL_NO_DISPLAY || lost_ || window.kind != NativeWindow::Android || !window.window)
      return false;
    releaseWindow();
    ANativeWindow* wnd = (ANativeWindow*)window.window;
    //the window's buffers in the config's pixel format
    EGLint format = 0;
    eglGetConfigAttrib(display_, config_, EGL_NATIVE_VISUAL_ID, &format);
    ANativeWindow_setBuffersGeometry(wnd, 0, 0, format);
    surface_ = eglCreateWindowSurface(display_, config_, wnd, nullptr);
    if (surface_ == EGL_NO_SURFACE){
      TR_ERROR("Can't create the EGL window surface (0x%x)", eglGetError());
      return false;
    }
    if (!eglMakeCurrent(display_, surface_, surface_, ctx_)){
      checkLost("eglMakeCurrent");
      TR_ERROR("Unable to make the EGL context current");
      eglDestroySurface(display_, surface_);
      surface_ = EGL_NO_SURFACE;
      return false;
    }
    eglQuerySurface(display_, surface_, EGL_WIDTH, &width);
    eglQuerySurface(display_, surface_, EGL_HEIGHT, &height);
    return true;
  }

  bool isLost() const override {return lost_;}

  void swapBuffers() override{
    if (surface_ != EGL_NO_SURFACE && !eglSwapBuffers(display_, surface_))
      checkLost("eglSwapBuffers");
  }

  void setSwapInterval(int interval) override{
    if (display_ != EGL_NO_DISPLAY)
      eglSwapInterval(display_, interval);
  }

  void* getProcAddress(const char* name) override{
    return (void*)eglGetProcAddress(name);
  }

private:
  //! notes a lost context after a failed EGL call
  void checkLost(const char* call){
    TR_USE(CGE_Renderer_GL);
    EGLint error = eglGetError();
    if (error == EGL_CONTEXT_LOST){
      TR_WARN("%s: the EGL context was lost", call);
      lost_ = true;
    }
    else
      TR_ERROR("%s failed (0x%x)", call, error);
  }

  EGLDisplay display_;
  EGLConfig config_;
  //! the window's surface, or EGL_NO_SURFACE without a window
  EGLSurface surface_;
  //! the 1x1 pbuffer the context is current on without a window (none if surfaceless)
  EGLSurface idle_;
  EGLContext ctx_;
  bool surfaceless_;
  bool lost_;
};

}

std::unique_ptr<GLContext> GLContext::create(){
  return std::make_unique<EGLContextImpl>();
}

#endif
