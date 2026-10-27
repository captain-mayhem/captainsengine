#if defined(__EMSCRIPTEN__)
#include <string>

#include <emscripten/html5.h>

#include "GLContext.h"
#include "../../io/Tracing.h"

using namespace CGE::Gfx;

TR_CHANNEL_EXT(CGE_Renderer_GL);

namespace{

//! a WebGL 2 context (OpenGL ES 3.0) on a canvas
/*! The browser presents the canvas when the frame's callback returns, so
    swapping does nothing, and requestAnimationFrame paces the frames. */
class WebGLContextImpl : public GLContext{
public:
  WebGLContextImpl() : ctx_(0) {}
  ~WebGLContextImpl() {shutdown();}

  bool init(const NativeWindow& window, const DeviceDesc& desc, int& width, int& height) override{
    TR_USE(CGE_Renderer_GL);
    if (window.kind != NativeWindow::Emscripten || !window.canvas){
      TR_ERROR("WebGL needs a canvas");
      return false;
    }
    canvas_ = window.canvas;
    EmscriptenWebGLContextAttributes attribs;
    emscripten_webgl_init_context_attributes(&attribs);
    attribs.majorVersion = 2;
    attribs.minorVersion = 0;
    //the page behind the canvas mustn't shine through
    attribs.alpha = false;
    attribs.depth = desc.depthBits > 0;
    attribs.stencil = desc.stencilBits > 0;
    attribs.antialias = false;
    attribs.premultipliedAlpha = false;
    attribs.preserveDrawingBuffer = false;
    attribs.powerPreference = EM_WEBGL_POWER_PREFERENCE_HIGH_PERFORMANCE;
    //the engine sees context loss as an error, not as a hint to wait
    attribs.failIfMajorPerformanceCaveat = false;
    ctx_ = emscripten_webgl_create_context(canvas_.c_str(), &attribs);
    if (ctx_ <= 0){
      TR_ERROR("Can't create a WebGL 2 context on %s (error %d)", canvas_.c_str(), (int)ctx_);
      ctx_ = 0;
      return false;
    }
    if (emscripten_webgl_make_context_current(ctx_) != EMSCRIPTEN_RESULT_SUCCESS){
      TR_ERROR("Can't make the WebGL context current");
      return false;
    }
    emscripten_webgl_get_drawing_buffer_size(ctx_, &width, &height);
    return true;
  }

  void shutdown() override{
    if (!ctx_)
      return;
    emscripten_webgl_make_context_current(0);
    emscripten_webgl_destroy_context(ctx_);
    ctx_ = 0;
  }

  void swapBuffers() override{
  }

  void setSwapInterval(int interval) override{
  }

  void* getProcAddress(const char* name) override{
    return emscripten_webgl_get_proc_address(name);
  }

private:
  std::string canvas_;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx_;
};

}

std::unique_ptr<GLContext> GLContext::create(){
  return std::make_unique<WebGLContextImpl>();
}

#endif
