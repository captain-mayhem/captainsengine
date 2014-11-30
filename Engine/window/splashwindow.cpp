#include "splashwindow.h"
#include "window.h"
#include "image/image.h"
#if defined(LINUX) && !defined(NO_X11)
#include "nativeLinux.h"
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#endif
#include "../io/Tracing.h"
#include <cstdlib>
#include <cstring>

using namespace CGE;

SplashWindow::SplashWindow(AppWindow* parent) {
#ifdef WIN32
  memset(&m_wc, 0, sizeof(m_wc));
  m_wc.lpfnWndProc = DefWindowProc;
  m_wc.hInstance = GetModuleHandle(NULL);
  m_wc.hIcon = NULL;// LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_SPLASHSCREEN))
  m_wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  m_wc.lpszClassName = "splashwindow";
  RegisterClass(&m_wc);

  HWND parentwnd = (HWND)(parent->getHandle());
  m_window = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST, "splashwindow", NULL, WS_POPUP | WS_VISIBLE, 0, 0, 0, 0, parentwnd, NULL, GetModuleHandle(NULL), NULL);
#elif defined(LINUX) && !defined(NO_X11)
  X11Window* pw = (X11Window*)parent;
  m_parent = pw;
#endif
}

SplashWindow::~SplashWindow(){
#ifdef WIN32
  DestroyWindow(m_window);
  UnregisterClass("splashwindow", GetModuleHandle(NULL));
#elif defined(LINUX) && !defined(NO_X11)
  XUnmapWindow(m_parent->getDisplay(), m_window);
#endif
}

void SplashWindow::show(Image& img){
#ifdef WIN32
  BITMAPINFO bminfo;
  ZeroMemory(&bminfo, sizeof(bminfo));
  bminfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bminfo.bmiHeader.biWidth = img.getWidth();
  bminfo.bmiHeader.biHeight = -((LONG)img.getHeight());
  bminfo.bmiHeader.biPlanes = 1;
  bminfo.bmiHeader.biBitCount = img.getNumChannels() * 8;
  bminfo.bmiHeader.biCompression = BI_RGB;

  HDC screen = GetDC(NULL);
  void* image;
  HBITMAP bmp = CreateDIBSection(screen, &bminfo, DIB_RGB_COLORS, &image, NULL, 0);

  unsigned char* tgt = (unsigned char*)image;
  unsigned char* src = (unsigned char*)img.getData();
  int srcrowsize = img.getRowSpan();
  int tgtrowsize = ((srcrowsize + 3) >> 2) << 2;
  for (unsigned j = 0; j < img.getHeight(); ++j){
    for (unsigned i = 0; i < img.getWidth(); ++i){
      unsigned char* tgtdata = tgt + j*tgtrowsize + i*img.getNumChannels();
      unsigned char* srcdata = src + j*srcrowsize + i*img.getNumChannels();
      if (img.getNumChannels() > 3){
        //premultiplied alpha
        tgtdata[0] = (unsigned char)((((float)srcdata[2]) / 255 * ((float)srcdata[3]) / 255) * 255);
        tgtdata[1] = (unsigned char)((((float)srcdata[1]) / 255 * ((float)srcdata[3]) / 255) * 255);
        tgtdata[2] = (unsigned char)((((float)srcdata[0]) / 255 * ((float)srcdata[3]) / 255) * 255);
        tgtdata[3] = srcdata[3];
      }
      else{
        tgtdata[0] = srcdata[2];
        tgtdata[1] = srcdata[1];
        tgtdata[2] = srcdata[0];
      }
    }
  }

  BITMAP bm;
  GetObject(bmp, sizeof(bm), &bm);
  SIZE sizeSplash = { bm.bmWidth, bm.bmHeight };
  POINT position;
  POINT zero = { 0 };
  HMONITOR primMon = MonitorFromPoint(zero, MONITOR_DEFAULTTOPRIMARY);
  MONITORINFO monInfo = { 0 };
  monInfo.cbSize = sizeof(monInfo);
  GetMonitorInfo(primMon, &monInfo);
  const RECT & rect = monInfo.rcWork;
  position.x = rect.left + (rect.right - rect.left - sizeSplash.cx) / 2;
  position.y = rect.top + (rect.bottom - rect.top - sizeSplash.cy) / 2;
  HDC mem = CreateCompatibleDC(screen);
  HBITMAP oldBmp = (HBITMAP)SelectObject(mem, bmp);


  BLENDFUNCTION blend = { 0 };
  blend.BlendOp = AC_SRC_OVER;
  blend.SourceConstantAlpha = 255;
  blend.AlphaFormat = AC_SRC_ALPHA;
  UpdateLayeredWindow(m_window, screen, &position, &sizeSplash, mem, &zero, RGB(0, 0, 0), &blend, img.getNumChannels() <= 3 ? ULW_OPAQUE : ULW_ALPHA);

  SelectObject(mem, oldBmp);
  DeleteDC(mem);
  ReleaseDC(NULL, screen);
#elif defined(LINUX) && !defined(NO_X11)
  unsigned char* data = (unsigned char*)malloc(img.getWidth()*img.getHeight()*4);
  if (img.getNumChannels() == 4){
    //premultiplied alpha
    for (int i = 0; i < img.getHeight(); ++i){
      for (int j = 0; j < img.getWidth(); ++j){
        unsigned char* tgt = data+i*img.getWidth()*4+j*4;
        unsigned char* src = img.getData()+i*img.getRowSpan()+j*img.getNumChannels();
        tgt[0] = (unsigned char)((((float)src[2]) / 255 * ((float)src[3]) / 255) * 255);
        tgt[1] = (unsigned char)((((float)src[1]) / 255 * ((float)src[3]) / 255) * 255);
        tgt[2] = (unsigned char)((((float)src[0]) / 255 * ((float)src[3]) / 255) * 255);
        tgt[3] = src[3];
      }
    }
  }
  else if (img.getNumChannels() == 3){
    //we need four byte packing
    for (int i = 0; i < img.getHeight(); ++i){
      for (int j = 0; j < img.getWidth(); ++j){
        unsigned char* tgt = data+i*img.getWidth()*4+j*4;
        unsigned char* src = img.getData()+i*img.getRowSpan()+j*img.getNumChannels();
        tgt[0] = src[0];
        tgt[1] = src[1];
        tgt[2] = src[2];
        tgt[3] = 255;
      }
    }
  }
  int bpp = img.getNumChannels()*8;
  XVisualInfo result;
  if(!XMatchVisualInfo(m_parent->getDisplay(), XDefaultScreen(m_parent->getDisplay()), bpp, TrueColor, &result)){
    printf("No matching visual!\n");
  }
  Visual* visual = result.visual;
  //printf("visual %i depth\n", result.depth);

  XSetWindowAttributes attribs;
  attribs.colormap = XCreateColormap(m_parent->getDisplay(), XDefaultRootWindow(m_parent->getDisplay()), visual, AllocNone);
  attribs.background_pixel = 0;
  attribs.border_pixel = 0;
  m_window = XCreateWindow(m_parent->getDisplay(), RootWindow(m_parent->getDisplay(), m_parent->getScreen()), 0, 0, img.getWidth(), img.getHeight(), 0, bpp, InputOutput, visual, CWBackPixel|CWColormap|CWBorderPixel, &attribs);
  //m_window = XCreateSimpleWindow(m_parent->getDisplay(), RootWindow(m_parent->getDisplay(), m_parent->getScreen()), 0, 0, img.getWidth(), img.getHeight(), 0, BlackPixel(m_parent->getWindow(), m_parent->getScreen()), WhitePixel(m_parent->getWindow(), m_parent->getScreen()));
  Atom type = XInternAtom(m_parent->getDisplay(), "_NET_WM_WINDOW_TYPE", False);
  Atom value = XInternAtom(m_parent->getDisplay(), "_NET_WM_WINDOW_TYPE_SPLASH", False);
  XChangeProperty(m_parent->getDisplay(), m_window, type, XA_ATOM, 32, PropModeReplace, (unsigned char*)&value, 1);
#
  Screen* scn = DefaultScreenOfDisplay(m_parent->getDisplay());
  XImage* ximg = XCreateImage(m_parent->getDisplay(), visual,
      bpp, ZPixmap, 0, (char*)data, img.getWidth(), img.getHeight(), 8, img.getWidth()*4);
  if (ximg == NULL){
    printf("Splash would crash!\n");
    return;
  }
  Pixmap pm = XCreatePixmap(m_parent->getDisplay(), m_window, img.getWidth(), img.getHeight(), img.getNumChannels()*8);
  GC gc = XCreateGC(m_parent->getDisplay(), pm, 0, NULL);
  XPutImage(m_parent->getDisplay(), pm, gc, ximg, 0, 0, 0, 0, img.getWidth(), img.getHeight());
  XFreeGC(m_parent->getDisplay(), gc);


  //XResizeWindow(m_parent->getDisplay(), m_window, img.getWidth(), img.getHeight());
  XSetWindowBackgroundPixmap(m_parent->getDisplay(), m_window, pm);
  //XFreePixmap(m_parent->getDisplay(), pm);
  //XDestroyImage(ximg);
  XClearWindow(m_parent->getDisplay(), m_window);
  Atom wmDeleteMessage = XInternAtom(m_parent->getDisplay(), "WM_DELETE_WINDOW", False);
  XSetWMProtocols(m_parent->getDisplay(), m_window, &wmDeleteMessage, 1);
  XMapWindow(m_parent->getDisplay(), m_window);
  XFlush(m_parent->getDisplay());
  free(data);
#endif
}
