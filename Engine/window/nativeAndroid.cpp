#if defined(ANDROID)
#include "../system/engine.h"
#include "../renderer/GL2/GL2Renderer.h"
#include "nativeAndroid.h"

using namespace CGE;

TR_CHANNEL_EXT(CGE_Window)

AndroidWindow::AndroidWindow(CGE::Renderer* renderer) : AppWindow(renderer){
  m_display = EGL_NO_DISPLAY;
  m_surface = EGL_NO_SURFACE;
}

void AndroidWindow::init(const std::string& name){
  TR_USE(CGE_Window);
  TR_INFO("Initializing window");
  
  const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
			EGL_DEPTH_SIZE, 8,
            EGL_NONE
    };
    EGLint w, h, dummy, format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, 0, 0);

    eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    ANativeWindow_setBuffersGeometry(CGE::Engine::instance()->getApp()->window, 0, 0, format);
    
    const EGLint surfAttribs[] ={
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE
    };

    surface = eglCreateWindowSurface(display, config, CGE::Engine::instance()->getApp()->window, NULL);
    context = eglCreateContext(display, config, NULL, surfAttribs);

    m_display = display;
    if (renderer_->getRenderType() == CGE::OpenGL2)
    static_cast< CGE::GL2Renderer* >(renderer_)->setContext(context);
  else
    TR_ERROR("Unsupported renderer");
    m_surface = surface;

  renderer_->initContext(this);

    eglQuerySurface(display, surface, EGL_WIDTH, &width_);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height_);
  
  renderer_->resizeScene(width_, height_);
}

void AndroidWindow::kill(){
  TR_USE(CGE_Window);
  TR_INFO("Killing window");
  
   if (m_display != EGL_NO_DISPLAY) {
       renderer_->killContext();
     
        if (m_surface != EGL_NO_SURFACE) {
            eglDestroySurface(m_display, m_surface);
        }
        eglTerminate(m_display);
    }
    m_display = EGL_NO_DISPLAY;
    m_surface = EGL_NO_SURFACE;
}

#endif