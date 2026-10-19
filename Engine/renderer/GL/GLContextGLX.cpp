#if defined(LINUX)
#include <string>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>

#include "GLContext.h"
#include "../../io/Tracing.h"

using namespace CGE::Gfx;

TR_CHANNEL_EXT(CGE_Renderer_GL);

namespace{

class GLXContextImpl : public GLContext{
public:
  GLXContextImpl() : disp_(nullptr), win_(0), screen_(0), visual_(nullptr), ctx_(nullptr) {}
  ~GLXContextImpl() {shutdown();}

  unsigned long chooseX11Visual(void* display, int screen, const DeviceDesc& desc) override{
    TR_USE(CGE_Renderer_GL);
    Display* disp = (Display*)display;
    int attribs[] = {GLX_RGBA, GLX_DOUBLEBUFFER,
      GLX_RED_SIZE, 4,
      GLX_GREEN_SIZE, 4,
      GLX_BLUE_SIZE, 4,
      GLX_DEPTH_SIZE, desc.depthBits,
      GLX_STENCIL_SIZE, desc.stencilBits,
      None};
    releaseVisual();
    visual_ = glXChooseVisual(disp, screen, attribs);
    if (!visual_){
      TR_ERROR("Got no double-buffered visual");
      return 0;
    }
    disp_ = disp;
    return visual_->visualid;
  }

  bool init(const NativeWindow& window, const DeviceDesc& desc, int& width, int& height) override{
    TR_USE(CGE_Renderer_GL);
    if (window.kind != NativeWindow::X11 || !window.display || !window.window){
      TR_ERROR("GLX needs an X11 window");
      return false;
    }
    disp_ = (Display*)window.display;
    win_ = (Window)window.window;
    screen_ = window.screen;
    XWindowAttributes attr;
    XGetWindowAttributes(disp_, win_, &attr);
    if (!visual_ || visual_->visualid != XVisualIDFromVisual(attr.visual)){
      //the window wasn't created with chooseX11Visual's visual: use its own
      releaseVisual();
      XVisualInfo templ;
      templ.visualid = XVisualIDFromVisual(attr.visual);
      int count;
      visual_ = XGetVisualInfo(disp_, VisualIDMask, &templ, &count);
      if (!visual_){
        TR_ERROR("The window's visual is unknown");
        return false;
      }
    }
    ctx_ = glXCreateContext(disp_, visual_, nullptr, True);
    if (!ctx_){
      TR_ERROR("Can't create the GLX context");
      return false;
    }
    if (!glXMakeCurrent(disp_, win_, ctx_)){
      TR_ERROR("Can't activate the GLX context");
      return false;
    }
    width = attr.width;
    height = attr.height;
    return true;
  }

  void shutdown() override{
    TR_USE(CGE_Renderer_GL);
    if (ctx_){
      if (!glXMakeCurrent(disp_, None, nullptr))
        TR_ERROR("Release of GL context failed");
      glXDestroyContext(disp_, ctx_);
      ctx_ = nullptr;
    }
    releaseVisual();
  }

  void swapBuffers() override{
    glXSwapBuffers(disp_, win_);
  }

  void setSwapInterval(int interval) override{
    TR_USE(CGE_Renderer_GL);
    typedef void (*SwapIntervalEXTFunc)(Display* dpy, GLXDrawable drawable, int interval);
    typedef int (*SwapIntervalMESAFunc)(unsigned interval);
    SwapIntervalEXTFunc swapIntervalEXT = (SwapIntervalEXTFunc)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalEXT");
    SwapIntervalMESAFunc swapIntervalMESA = (SwapIntervalMESAFunc)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalMESA");
    //Mesa under XWayland (WSLg) lists the swap control extensions only on the client side
    std::string extensions = std::string(glXQueryExtensionsString(disp_, screen_)) + " " + glXGetClientString(disp_, GLX_EXTENSIONS);
    if (swapIntervalEXT && extensions.find("GLX_EXT_swap_control") != std::string::npos)
      swapIntervalEXT(disp_, win_, interval);
    else if (swapIntervalMESA && extensions.find("GLX_MESA_swap_control") != std::string::npos)
      swapIntervalMESA(interval);
    else
      TR_WARN("No GLX swap control extension, vsync can't be set");
  }

private:
  void releaseVisual(){
    if (visual_)
      XFree(visual_);
    visual_ = nullptr;
  }

  Display* disp_;
  Window win_;
  int screen_;
  XVisualInfo* visual_;
  GLXContext ctx_;
};

}

std::unique_ptr<GLContext> GLContext::create(){
  return std::make_unique<GLXContextImpl>();
}

#endif
