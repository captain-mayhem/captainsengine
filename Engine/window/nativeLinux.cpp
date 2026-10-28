#if defined(LINUX)
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#include <algorithm>
#include <chrono>
#include <climits>
#include <clocale>
#include <thread>
#include <cstdlib>
#include <cstring>
#include <string>
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
  im_ = nullptr;
  ic_ = nullptr;
  wmProtocols_ = None;
  wmDelete_ = None;
  clipboard_ = None;
  utf8String_ = None;
  targets_ = None;
  text_ = None;
  incr_ = None;
  clipboardProperty_ = None;
  ownsClipboard_ = false;
}

namespace{

//! ISO 8859-1 (the selection's STRING) to UTF-8
std::string latin1ToUtf8(const std::string& latin1){
  std::string ret;
  for (unsigned char ch : latin1){
    if (ch < 0x80)
      ret += (char)ch;
    else{
      ret += (char)(0xC0 | (ch >> 6));
      ret += (char)(0x80 | (ch & 0x3F));
    }
  }
  return ret;
}

//! UTF-8 to ISO 8859-1; other characters become '?'
std::string utf8ToLatin1(const std::string& utf8){
  std::string ret;
  for (size_t i = 0; i < utf8.size(); ++i){
    unsigned char ch = (unsigned char)utf8[i];
    if (ch < 0x80)
      ret += (char)ch;
    else if ((ch & 0xE0) == 0xC0 && i+1 < utf8.size()){
      unsigned code = ((ch & 0x1F) << 6) | ((unsigned char)utf8[i+1] & 0x3F);
      ret += code < 0x100 ? (char)code : '?';
      ++i;
    }
    else if ((ch & 0xC0) != 0x80)
      ret += '?';
  }
  return ret;
}

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
  //PropertyChangeMask for the clipboard's incremental transfers
  attr.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask
    | LeaveWindowMask | FocusChangeMask | PropertyChangeMask;
  windowedPixels(width_, height_);
  win_ = XCreateWindow(disp_, RootWindow(disp_, vi->screen), 0, 0,
      width_, height_, 0, vi->depth, InputOutput, vi->visual,
      CWBorderPixel | CWColormap | CWEventMask, &attr);
  wmProtocols_ = XInternAtom(disp_, "WM_PROTOCOLS", False);
  wmDelete_ = XInternAtom(disp_, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(disp_, win_, &wmDelete_, 1);
  clipboard_ = XInternAtom(disp_, "CLIPBOARD", False);
  utf8String_ = XInternAtom(disp_, "UTF8_STRING", False);
  targets_ = XInternAtom(disp_, "TARGETS", False);
  text_ = XInternAtom(disp_, "TEXT", False);
  incr_ = XInternAtom(disp_, "INCR", False);
  clipboardProperty_ = XInternAtom(disp_, "CGE_CLIPBOARD", False);
  ownsClipboard_ = false;
  clipboardText_.clear();
  XSetStandardProperties(disp_, win_, name.c_str(), name.c_str(),
      None, nullptr, 0, nullptr);
  //an input method for the typed characters (keyboard layouts, dead keys,
  //compose), opened in the user's locale; the C library's locale stays
  std::string locale = setlocale(LC_CTYPE, nullptr);
  setlocale(LC_CTYPE, "");
  XSetLocaleModifiers("");
  im_ = XOpenIM(disp_, nullptr, nullptr, nullptr);
  if (!im_){
    //the one named by XMODIFIERS isn't running: X's own
    XSetLocaleModifiers("@im=none");
    im_ = XOpenIM(disp_, nullptr, nullptr, nullptr);
  }
  setlocale(LC_CTYPE, locale.c_str());
  if (im_){
    ic_ = XCreateIC(im_, XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
      XNClientWindow, win_, XNFocusWindow, win_, nullptr);
  }
  if (ic_){
    //the events the input method needs, too
    long filter = 0;
    XGetICValues(ic_, XNFilterEvents, &filter, nullptr);
    XSelectInput(disp_, win_, attr.event_mask | filter);
  }
  else
    TR_WARN("No X input method: characters are typed without the keyboard layout's dead keys");
  //before the window is mapped, fullscreen is a property
  if (fullscreen_){
    fullscreen_ = false;
    setFullscreen(true);
  }
  if (shown_)
    show(true);
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

void X11Window::windowedPixels(int& width, int& height) const{
  float scale = getContentScale();
  width = (std::min)((int)(windowedWidth_*scale+0.5f), DisplayWidth(disp_, screen_));
  height = (std::min)((int)(windowedHeight_*scale+0.5f), DisplayHeight(disp_, screen_));
}

void X11Window::setFullscreen(bool fullscreen){
  if (!win_ || fullscreen == fullscreen_){
    fullscreen_ = fullscreen;
    return;
  }
  fullscreen_ = fullscreen;
  Atom state = XInternAtom(disp_, "_NET_WM_STATE", False);
  Atom fullscreenState = XInternAtom(disp_, "_NET_WM_STATE_FULLSCREEN", False);
  if (!shown_){
    //the window manager reads it when the window is mapped
    if (fullscreen)
      XChangeProperty(disp_, win_, state, XA_ATOM, 32, PropModeReplace, (unsigned char*)&fullscreenState, 1);
    else
      XDeleteProperty(disp_, win_, state);
    return;
  }
  //a mapped window asks the window manager (EWMH)
  XEvent event;
  memset(&event, 0, sizeof(event));
  event.xclient.type = ClientMessage;
  event.xclient.window = win_;
  event.xclient.message_type = state;
  event.xclient.format = 32;
  event.xclient.data.l[0] = fullscreen ? 1 : 0; //_NET_WM_STATE_ADD or _REMOVE
  event.xclient.data.l[1] = fullscreenState;
  event.xclient.data.l[3] = 1; //from an application
  XSendEvent(disp_, RootWindow(disp_, screen_), False, SubstructureRedirectMask | SubstructureNotifyMask, &event);
  //the size may have changed in fullscreen (changeSize)
  if (!fullscreen){
    int width, height;
    windowedPixels(width, height);
    XResizeWindow(disp_, win_, width, height);
  }
  XFlush(disp_);
}

bool X11Window::isCloseRequest(const XEvent& event) const{
  return event.type == ClientMessage && event.xclient.message_type == wmProtocols_ &&
    (Atom)event.xclient.data.l[0] == wmDelete_;
}

void X11Window::setClipboardText(const std::string& text){
  if (!win_)
    return;
  clipboardText_ = text;
  XSetSelectionOwner(disp_, clipboard_, win_, CurrentTime);
  ownsClipboard_ = XGetSelectionOwner(disp_, clipboard_) == win_;
  if (!ownsClipboard_)
    clipboardText_.clear();
  XFlush(disp_);
}

bool X11Window::getClipboardText(std::string& text){
  if (!win_)
    return false;
  Window owner = XGetSelectionOwner(disp_, clipboard_);
  if (owner == None)
    return false;
  if (owner == win_){
    text = clipboardText_;
    return true;
  }
  Atom type;
  if (convertClipboard(utf8String_, text, type))
    return true;
  //clients that only know ISO 8859-1
  std::string latin1;
  if (!convertClipboard(XA_STRING, latin1, type))
    return false;
  text = latin1ToUtf8(latin1);
  return true;
}

bool X11Window::hasClipboardText() const{
  if (!win_)
    return false;
  Window owner = XGetSelectionOwner(disp_, clipboard_);
  return owner == win_ ? !clipboardText_.empty() : owner != None;
}

bool X11Window::waitForEvent(int type, XEvent& event){
  //the answer comes a few milliseconds later; the window's other events
  //stay in the queue
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now() + std::chrono::seconds(1);
  XFlush(disp_);
  while (!XCheckTypedWindowEvent(disp_, win_, type, &event)){
    if (std::chrono::steady_clock::now() > end)
      return false;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  return true;
}

bool X11Window::convertClipboard(Atom target, std::string& data, Atom& type){
  TR_USE(CGE_Window);
  XDeleteProperty(disp_, win_, clipboardProperty_);
  XConvertSelection(disp_, clipboard_, target, clipboardProperty_, win_, CurrentTime);
  XEvent event;
  do{
    if (!waitForEvent(SelectionNotify, event)){
      TR_WARN("The clipboard's owner doesn't answer");
      return false;
    }
  } while (event.xselection.selection != clipboard_);
  //the owner can't convert it
  if (event.xselection.property == None)
    return false;
  int format;
  unsigned long count, after;
  unsigned char* value = nullptr;
  if (XGetWindowProperty(disp_, win_, clipboardProperty_, 0, LONG_MAX/4, True, AnyPropertyType,
    &type, &format, &count, &after, &value) != Success)
    return false;
  if (type != incr_){
    data.assign((const char*)value, count);
    XFree(value);
    return true;
  }
  XFree(value);
  //an incremental transfer (large texts): deleting the property asked for
  //the first part; the owner writes the parts one after another, an empty
  //one last
  data.clear();
  while (true){
    if (!waitForEvent(PropertyNotify, event)){
      TR_WARN("The clipboard's owner stopped sending");
      return false;
    }
    if (event.xproperty.atom != clipboardProperty_ || event.xproperty.state != PropertyNewValue)
      continue;
    if (XGetWindowProperty(disp_, win_, clipboardProperty_, 0, LONG_MAX/4, True, AnyPropertyType,
      &type, &format, &count, &after, &value) != Success)
      return false;
    //an event from before a part was read (e.g. for the INCR property
    //itself): the property is gone, the next part hasn't come yet
    if (type == None){
      if (value)
        XFree(value);
      continue;
    }
    data.append((const char*)value, count);
    XFree(value);
    //the end is an empty part (which has a type)
    if (count == 0)
      return true;
  }
}

bool X11Window::handleSelectionEvent(const XEvent& event){
  TR_USE(CGE_Window);
  if (event.type == SelectionClear){
    //another client copied something
    if (event.xselectionclear.selection == clipboard_){
      ownsClipboard_ = false;
      clipboardText_.clear();
    }
    return true;
  }
  if (event.type != SelectionRequest)
    return false;
  const XSelectionRequestEvent& request = event.xselectionrequest;
  XEvent reply;
  memset(&reply, 0, sizeof(reply));
  reply.xselection.type = SelectionNotify;
  reply.xselection.requestor = request.requestor;
  reply.xselection.selection = request.selection;
  reply.xselection.target = request.target;
  reply.xselection.time = request.time;
  reply.xselection.property = None;
  //obsolete clients name no property
  Atom property = request.property != None ? request.property : request.target;
  if (request.selection == clipboard_ && ownsClipboard_){
    if (request.target == targets_){
      Atom targets[] = {targets_, utf8String_, text_, XA_STRING};
      XChangeProperty(disp_, request.requestor, property, XA_ATOM, 32, PropModeReplace,
        (unsigned char*)targets, sizeof(targets)/sizeof(targets[0]));
      reply.xselection.property = property;
    }
    else if (request.target == utf8String_ || request.target == text_ || request.target == XA_STRING){
      std::string data = request.target == XA_STRING ? utf8ToLatin1(clipboardText_) : clipboardText_;
      //in one request; larger texts would need an incremental transfer
      long maxSize = XExtendedMaxRequestSize(disp_);
      if (maxSize == 0)
        maxSize = XMaxRequestSize(disp_);
      if ((long)data.size() < maxSize*4 - 256){
        XChangeProperty(disp_, request.requestor, property, request.target == XA_STRING ? XA_STRING : utf8String_,
          8, PropModeReplace, (const unsigned char*)data.data(), (int)data.size());
        reply.xselection.property = property;
      }
      else
        TR_WARN("The clipboard's text is too large to send (%u bytes)", (unsigned)data.size());
    }
  }
  XSendEvent(disp_, request.requestor, False, NoEventMask, &reply);
  XFlush(disp_);
  return true;
}

void X11Window::kill(){
  TR_USE(CGE_Window);
  TR_INFO("Killing window");
  if (ic_)
    XDestroyIC(ic_);
  if (im_)
    XCloseIM(im_);
  ic_ = nullptr;
  im_ = nullptr;
  ownsClipboard_ = false;
  clipboardText_.clear();
  XCloseDisplay(disp_);
}


void X11Window::changeSize(int width, int height){
  windowedWidth_ = width;
  windowedHeight_ = height;
  //in fullscreen, the size is used when the window leaves it (setFullscreen)
  if (!win_ || fullscreen_)
    return;
  //the size goes to Engine::resize with ConfigureNotify
  int pixelWidth, pixelHeight;
  windowedPixels(pixelWidth, pixelHeight);
  XResizeWindow(disp_, win_, pixelWidth, pixelHeight);
}

void X11Window::show(bool doit){
  if (doit){
    XMapRaised(disp_, win_);
  }
  else
    XUnmapWindow(disp_, win_);
  AppWindow::show(doit);
}

void X11Window::setTitle(const std::string& title){
  if (!win_)
    return;
  //the window's and its icon's name, as at creation
  XSetStandardProperties(disp_, win_, title.c_str(), title.c_str(),
      None, nullptr, 0, nullptr);
  XFlush(disp_);
}


#endif
