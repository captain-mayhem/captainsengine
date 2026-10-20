#if defined(ANDROID)
#include <EGL/egl.h>
#include <android/native_window.h>

#include "GLContext.h"
#include "../../io/Tracing.h"

using namespace CGE::Gfx;

TR_CHANNEL_EXT(CGE_Renderer_GL);

namespace{

//! an OpenGL ES 2.0 context; becomes ES 3.0 with the Android port (Phase 1c)
class EGLContextImpl : public GLContext{
public:
  EGLContextImpl() : display_(EGL_NO_DISPLAY), surface_(EGL_NO_SURFACE), ctx_(EGL_NO_CONTEXT) {}
  ~EGLContextImpl() {shutdown();}

  bool init(const NativeWindow& window, const DeviceDesc& desc, int& width, int& height) override{
    TR_USE(CGE_Renderer_GL);
    if (window.kind != NativeWindow::Android || !window.window){
      TR_ERROR("EGL needs an Android window");
      return false;
    }
    ANativeWindow* wnd = (ANativeWindow*)window.window;
    const EGLint attribs[] = {
      EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
      EGL_BLUE_SIZE, 8,
      EGL_GREEN_SIZE, 8,
      EGL_RED_SIZE, 8,
      EGL_DEPTH_SIZE, desc.depthBits,
      EGL_STENCIL_SIZE, desc.stencilBits,
      EGL_NONE
    };
    display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display_ == EGL_NO_DISPLAY || !eglInitialize(display_, nullptr, nullptr)){
      TR_ERROR("Can't initialise EGL");
      return false;
    }
    EGLConfig config;
    EGLint numConfigs = 0;
    if (!eglChooseConfig(display_, attribs, &config, 1, &numConfigs) || numConfigs == 0){
      TR_ERROR("No suitable EGL config");
      return false;
    }
    EGLint format;
    eglGetConfigAttrib(display_, config, EGL_NATIVE_VISUAL_ID, &format);
    ANativeWindow_setBuffersGeometry(wnd, 0, 0, format);

    surface_ = eglCreateWindowSurface(display_, config, wnd, nullptr);
    const EGLint ctxAttribs[] = {
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE
    };
    ctx_ = eglCreateContext(display_, config, EGL_NO_CONTEXT, ctxAttribs);
    if (surface_ == EGL_NO_SURFACE || ctx_ == EGL_NO_CONTEXT){
      TR_ERROR("Can't create the EGL surface or context");
      return false;
    }
    if (eglMakeCurrent(display_, surface_, surface_, ctx_) == EGL_FALSE){
      TR_ERROR("Unable to eglMakeCurrent");
      return false;
    }
    TR_INFO("Made egl context current");
    eglQuerySurface(display_, surface_, EGL_WIDTH, &width);
    eglQuerySurface(display_, surface_, EGL_HEIGHT, &height);
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
    display_ = EGL_NO_DISPLAY;
    surface_ = EGL_NO_SURFACE;
    ctx_ = EGL_NO_CONTEXT;
  }

  void swapBuffers() override{
    eglSwapBuffers(display_, surface_);
  }

  void setSwapInterval(int interval) override{
    eglSwapInterval(display_, interval);
  }

  void* getProcAddress(const char* name) override{
    return (void*)eglGetProcAddress(name);
  }

private:
  EGLDisplay display_;
  EGLSurface surface_;
  EGLContext ctx_;
};

}

std::unique_ptr<GLContext> GLContext::create(){
  return std::make_unique<EGLContextImpl>();
}

#endif
