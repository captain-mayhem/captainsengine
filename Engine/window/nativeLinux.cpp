#if defined(LINUX)
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <cstdlib>
#include "../system/engine.h"
#include "../renderer/renderdevice.h"
#include "nativeLinux.h"

TR_CHANNEL_EXT(CGE_Window);

using namespace CGE;

X11Window::X11Window(Gfx::RenderDevice* device, const Gfx::DeviceDesc& desc) : AppWindow(device, desc){
  disp_ = nullptr;
  win_ = 0;
  screen_ = 0;
  contentScale_ = 0;
}

void X11Window::init(const std::string& name){
  TR_USE(CGE_Window);
  TR_INFO("Initializing window");

  disp_ = XOpenDisplay(0);
  screen_ = DefaultScreen(disp_);

  //the render device may need a particular visual for its context
  XVisualInfo templ;
  templ.screen = screen_;
  templ.visualid = device_ ? device_->chooseX11Visual(disp_, screen_, deviceDesc_) : 0;
  if (templ.visualid == 0)
    templ.visualid = XVisualIDFromVisual(DefaultVisual(disp_, screen_));
  int count;
  XVisualInfo* vi = XGetVisualInfo(disp_, VisualIDMask | VisualScreenMask, &templ, &count);
  if (vi == nullptr){
    EXIT2("No visual for the window");
    return;
  }

  XSetWindowAttributes attr;
  Colormap color = XCreateColormap(disp_, RootWindow(disp_, vi->screen),
      vi->visual, AllocNone);
  attr.colormap = color;
  attr.border_pixel = 0;
  
  if (shown_)
    applyResolution(); 
  
  if (fullscreen_){
    attr.override_redirect = True;
    attr.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask
      | LeaveWindowMask | FocusChangeMask;
    win_ = XCreateWindow(disp_, RootWindow(disp_, vi->screen), 0, 0,
        width_, height_, 0, vi->depth, InputOutput, vi->visual,
        CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect,
        &attr);
    XWarpPointer(disp_, None, win_, 0, 0, 0, 0, 0, 0);
    XMapRaised(disp_, win_);
    XGrabKeyboard(disp_, win_, True, GrabModeAsync, GrabModeAsync, CurrentTime);
    XGrabPointer(disp_, win_, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync,
        GrabModeAsync, win_, None, CurrentTime);
  }
  else{
    attr.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask
      | LeaveWindowMask | FocusChangeMask;
    win_ = XCreateWindow(disp_, RootWindow(disp_, vi->screen), 0, 0,
        width_, height_, 0, vi->depth, InputOutput, vi->visual,
        CWBorderPixel | CWColormap | CWEventMask, &attr);
    Atom wmDelete = XInternAtom(disp_, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(disp_, win_, &wmDelete, 1);
    XSetStandardProperties(disp_, win_, name.c_str(), name.c_str(), 
        None, nullptr, 0, nullptr);
    if (shown_)
      show(true);
  }
  XFree(vi);
}

Gfx::NativeWindow X11Window::getNativeWindow() const{
  Gfx::NativeWindow wnd;
  wnd.kind = Gfx::NativeWindow::X11;
  wnd.display = disp_;
  wnd.window = (uintptr_t)win_;
  wnd.screen = screen_;
  return wnd;
}

float X11Window::getContentScale() const{
  if (contentScale_ > 0)
    return contentScale_;
  contentScale_ = 1;
  //desktops that scale set the font DPI in the resource database
  const char* resources = disp_ ? XResourceManagerString(disp_) : nullptr;
  if (!resources)
    return contentScale_;
  XrmInitialize();
  XrmDatabase db = XrmGetStringDatabase(resources);
  char* type = nullptr;
  XrmValue value;
  if (db && XrmGetResource(db, "Xft.dpi", "Xft.Dpi", &type, &value) && value.addr){
    float dpi = (float)atof(value.addr);
    if (dpi > 0)
      contentScale_ = dpi/96.0f;
  }
  if (db)
    XrmDestroyDatabase(db);
  return contentScale_;
}

void X11Window::applyResolution(){
  if (fullscreen_){
    int videoMajor;
    int videoMinor;
    XF86VidModeQueryVersion(disp_, &videoMajor, &videoMinor);

    int numModes;
    XF86VidModeModeInfo** modes;
    XF86VidModeGetAllModeLines(disp_, screen_, &numModes, &modes);
    static bool first = true;
    if (first){
      prevDesktop_ = *modes[0];
      first = false;
    }
  
    //search for best resolution
    int bestMode = 0;
    for (int i = 0; i < numModes; i++){
      if (modes[i]->hdisplay == width_ && modes[i]->vdisplay == height_){
        bestMode = i;
      }
    }

    //Change video mode
    XF86VidModeSwitchToMode(disp_, screen_, modes[bestMode]);
    XF86VidModeSetViewPort(disp_, screen_, 0, 0);
    width_ = modes[bestMode]->hdisplay;
    height_ = modes[bestMode]->vdisplay;
    XFree(modes);
 
  }
}

void X11Window::kill(){
  TR_USE(CGE_Window);
  TR_INFO("Killing window");

  if (fullscreen_){
    XF86VidModeSwitchToMode(disp_, screen_, &prevDesktop_);
    XF86VidModeSetViewPort(disp_, screen_, 0, 0);
  }
  
  XCloseDisplay(disp_);
}


void X11Window::changeSize(int width, int height){
  AppWindow::changeSize(width, height);
  XResizeWindow(disp_, win_, width, height);
  applyResolution();
}

void X11Window::show(bool doit){
  if (doit){
    XMapRaised(disp_, win_);
  }
  else
    XUnmapWindow(disp_, win_);
  AppWindow::show(doit);
}


#endif
