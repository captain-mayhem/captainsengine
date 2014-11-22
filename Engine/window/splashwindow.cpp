#include "splashwindow.h"
#include "window.h"
#include "image/image.h"

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
#endif
}

SplashWindow::~SplashWindow(){
#ifdef WIN32
  DestroyWindow(m_window);
  UnregisterClass("splashwindow", GetModuleHandle(NULL));
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
#endif
}
