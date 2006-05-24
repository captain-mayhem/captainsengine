#ifdef UNIX
#include <GL/glx.h>
#include "../system/engine.h"
#include "../renderer/OGLrenderer.h"
//#include "../input/keyboard.h"
#include "nativeLinux.h"

namespace Windows{

X11Window::X11Window(::Graphics::Renderer* renderer) : AppWindow(renderer){
  disp_ = NULL;
  win_ = 0;
}

void X11Window::init(const std::string& name){
  System::Log << "Initializing window\n";

  disp_ = XOpenDisplay(0);
  screen_ = DefaultScreen(disp_);

  int videoMajor;
  int videoMinor;
  XF86VidModeQueryVersion(disp_, &videoMajor, &videoMinor);

  int numModes;
  XF86VidModeModeInfo** modes;
  XF86VidModeGetAllModeLines(disp_, screen_, &numModes, &modes);
  prevDesktop_ = *modes[0];
  
  //search for best resolution
  int bestMode = 0;
  for (int i = 0; i < numModes; i++){
    if (modes[i]->hdisplay == width_ && modes[i]->vdisplay == height_){
      bestMode = i;
    }
  }

  int attribs[] = {GLX_RGBA, GLX_DOUBLEBUFFER,
    GLX_RED_SIZE, 4,
    GLX_GREEN_SIZE, 4,
    GLX_BLUE_SIZE, 4,
    GLX_DEPTH_SIZE, 16,
    None};
  XVisualInfo* vi = glXChooseVisual(disp_, screen_, attribs);
  if (vi == NULL)
    EXIT2("Got no doublebuffered window");

  int glxMajor;
  int glxMinor;
  glXQueryVersion(disp_, &glxMajor, &glxMinor);
  
  XSetWindowAttributes attr;
  GLXContext glx = glXCreateContext(disp_, vi, 0, GL_TRUE);
  dynamic_cast< ::Graphics::OGLRenderer* >(renderer_)->setGLX(glx);
  Colormap color = XCreateColormap(disp_, RootWindow(disp_, vi->screen),
      vi->visual, AllocNone);
  attr.colormap = color;
  attr.border_pixel = 0;
  
  if (fullscreen_){
    //Change video mode
    XF86VidModeSwitchToMode(disp_, screen_, modes[bestMode]);
    XF86VidModeSetViewPort(disp_, screen_, 0, 0);
    width_ = modes[bestMode]->hdisplay;
    height_ = modes[bestMode]->vdisplay;
    XFree(modes);
    
    attr.override_redirect = True;
    attr.event_mask = ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask;
    win_ = XCreateWindow(disp_, RootWindow(disp_, vi->screen), 0, 0,
        width_, height_, 0, vi->depth, InputOutput, vi->visual,
        CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect,
        &attr);
    XWarpPointer(disp_, None, win_, 0, 0, 0, 0, 0, 0);
    XMapRaised(disp_, win_);
    XGrabKeyboard(disp_, win_, True, GrabModeAsync, GrabModeAsync, CurrentTime);
    XGrabPointer(disp_, win_, True, ButtonPressMask, GrabModeAsync,
        GrabModeAsync, win_, None, CurrentTime);
  }
  else{
    attr.event_mask = ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask;
    win_ = XCreateWindow(disp_, RootWindow(disp_, vi->screen), 0, 0,
        width_, height_, 0, vi->depth, InputOutput, vi->visual,
        CWBorderPixel | CWColormap | CWEventMask, &attr);
    Atom wmDelete = XInternAtom(disp_, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(disp_, win_, &wmDelete, 1);
    XSetStandardProperties(disp_, win_, name.c_str(), name.c_str(), 
        None, NULL, 0, NULL);
    XMapRaised(disp_, win_);
  }

  glx = NULL;
  renderer_->initContext(this);

}

void X11Window::kill(){
  ::System::Log << "Killing window\n";

  renderer_->killContext();

  if (fullscreen_){
    XF86VidModeSwitchToMode(disp_, screen_, &prevDesktop_);
    XF86VidModeSetViewPort(disp_, screen_, 0, 0);
  }
  
  XCloseDisplay(disp_);
}

}
#endif
