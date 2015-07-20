#include "util.h"
#include <sstream>
#include <map>
#include <cassert>
#include "funcids.h"
#include "wgl_run.h"
#include "ogl.h"

#ifdef WIN32
#define WINGDIAPI __declspec(dllexport)
#define WINAPI __stdcall
#else
#define WINGDIAPI
#define WINAPI
#endif
#define WGL_API WINGDIAPI
typedef int BOOL;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned char BYTE;

DECLARE_HANDLE(HGLRC);
typedef struct tagPIXELFORMATDESCRIPTOR {
  WORD nSize;
  WORD nVersion;
  DWORD dwFlags;
  BYTE iPixelType;
  BYTE cColorBits;
  BYTE cRedBits;
  BYTE cRedShift;
  BYTE cGreenBits;
  BYTE cGreenShift;
  BYTE cBlueBits;
  BYTE cBlueShift;
  BYTE cAlphaBits;
  BYTE cAlphaShift;
  BYTE cAccumBits;
  BYTE cAccumRedBits;
  BYTE cAccumGreenBits;
  BYTE cAccumBlueBits;
  BYTE cAccumAlphaBits;
  BYTE cDepthBits;
  BYTE cStencilBits;
  BYTE cAuxBuffers;
  BYTE iLayerType;
  BYTE bReserved;
  DWORD dwLayerMask;
  DWORD dwVisibleMask;
  DWORD dwDamageMask;
} PIXELFORMATDESCRIPTOR;

extern "C"{
  WINGDIAPI int   WINAPI wglChoosePixelFormat(HDC hdc, const PIXELFORMATDESCRIPTOR *ppfd);
  WINGDIAPI HGLRC WINAPI wglCreateContext(HDC);
  WINGDIAPI BOOL  WINAPI wglDeleteContext(HGLRC);
  WINGDIAPI int   WINAPI wglDescribePixelFormat(HDC hdc, int iPixelFormat, unsigned nBytes, PIXELFORMATDESCRIPTOR *ppfd);
  WINGDIAPI int   WINAPI wglGetPixelFormat(HDC hdc);
  WINGDIAPI BOOL  WINAPI wglMakeCurrent(HDC, HGLRC);
  WINGDIAPI BOOL  WINAPI wglSetPixelFormat(HDC hdc, int format, const PIXELFORMATDESCRIPTOR * ppfd);
  WINGDIAPI BOOL  WINAPI wglShareLists(HGLRC, HGLRC);
  WINGDIAPI BOOL  WINAPI wglSwapBuffers(HDC);
};

static void cleanup();

struct WGLData{
  WGLData() : hdccount(-1), mHglrcCount(-1){
    atexit(cleanup);
  }
  int hdccount;
  std::map<HDC, int> hdcs;
  int mHglrcCount;
  std::map<HGLRC, int> mHglrcs;
};

static WGLData* gData = NULL;

void cleanup(){
  delete gData;
}

WGL_API int WINAPI wglChoosePixelFormat(HDC hdc, const PIXELFORMATDESCRIPTOR *ppfd){
  gData = new WGLData;
  wgl_init();
  ogl_init();
  wgl_add_DC(hdc);
  if (gData->hdcs[hdc] == 0)
    gData->hdcs[hdc] = ++gData->hdccount;
  std::ostringstream out;
  int num = gData->hdcs[hdc];
  out << WGLCHOOSEPIXELFORMAT << " " << num;
  out << " " << ppfd->nSize << " " << ppfd->nVersion << " " << ppfd->dwFlags << " " << ppfd->iPixelType << " " << ppfd->cColorBits << " " << ppfd->cRedBits << " " << ppfd->cRedShift << " " << ppfd->cGreenBits << " " << ppfd->cGreenShift << " " << ppfd->cBlueBits << " " << ppfd->cBlueShift << " " << ppfd->cAlphaBits << " " << ppfd->cAlphaShift << " " << ppfd->cAccumBits << " " << ppfd->cAccumRedBits << " " << ppfd->cAccumGreenBits << " " << ppfd->cAccumBlueBits << " " << ppfd->cAccumAlphaBits << " " << ppfd->cDepthBits << " " << ppfd->cStencilBits << " " << ppfd->cAuxBuffers << " " << ppfd->iLayerType << " " << ppfd->bReserved << " " << ppfd->dwLayerMask << " " << ppfd->dwVisibleMask << " " << ppfd->dwDamageMask;
  int ret = (int)wgl_interpret(out.str());
  return ret;
}

WGL_API HGLRC WINAPI wglCreateContext(HDC hdc){
  std::ostringstream out;
  int num = gData->hdcs[hdc];
  out << WGLCREATECONTEXT << " " << num;
  HGLRC rc = (HGLRC)wgl_interpret(out.str());
  gData->mHglrcs[rc] = ++gData->mHglrcCount;
  return rc;
}

WGL_API BOOL WINAPI wglDeleteContext(HGLRC hglrc){
  std::ostringstream out;
  int num = gData->mHglrcs[hglrc];
  out << WGLDELETECONTEXT << " " << num;
  BOOL rc = (BOOL)wgl_interpret(out.str());
  return rc;
}

WINGDIAPI int WINAPI wglDescribePixelFormat(HDC hdc, int iPixelFormat, unsigned nBytes, PIXELFORMATDESCRIPTOR *ppfd){
  std::ostringstream out;
  int num = gData->hdcs[hdc];
  out << WGLDESCRIBEPIXELFORMAT << " " << num << " " << iPixelFormat << " " << nBytes << " " << ppfd;
  int ret = (int)wgl_interpret(out.str());
  return ret;
}

WGL_API int WINAPI wglGetPixelFormat(HDC hdc){
  std::ostringstream out;
  int num = gData->hdcs[hdc];
  out << WGLGETPIXELFORMAT << " " << num;
  int ret = (int)wgl_interpret(out.str());
  return ret;
}

WGL_API BOOL  WINAPI wglMakeCurrent(HDC hdc, HGLRC rc){
  std::ostringstream out;
  int hdcnum = gData->hdcs[hdc];
  int hglrcnum = gData->mHglrcs[rc];
  out << WGLMAKECURRENT << " " << hdcnum << " " << hglrcnum;
  BOOL ret = (BOOL)wgl_interpret(out.str());
  return ret;
}

WGL_API BOOL  WINAPI wglSetPixelFormat(HDC hdc, int format, const PIXELFORMATDESCRIPTOR * ppfd){
  std::ostringstream out;
  int num = gData->hdcs[hdc];
  out << WGLSETPIXELFORMAT << " " << num << " " << format;
  out << " " << ppfd->nSize << " " << ppfd->nVersion << " " << ppfd->dwFlags << " " << ppfd->iPixelType << " " << ppfd->cColorBits << " " << ppfd->cRedBits << " " << ppfd->cRedShift << " " << ppfd->cGreenBits << " " << ppfd->cGreenShift << " " << ppfd->cBlueBits << " " << ppfd->cBlueShift << " " << ppfd->cAlphaBits << " " << ppfd->cAlphaShift << " " << ppfd->cAccumBits << " " << ppfd->cAccumRedBits << " " << ppfd->cAccumGreenBits << " " << ppfd->cAccumBlueBits << " " << ppfd->cAccumAlphaBits << " " << ppfd->cDepthBits << " " << ppfd->cStencilBits << " " << ppfd->cAuxBuffers << " " << ppfd->iLayerType << " " << ppfd->bReserved << " " << ppfd->dwLayerMask << " " << ppfd->dwVisibleMask << " " << ppfd->dwDamageMask;
  BOOL ret = (BOOL)wgl_interpret(out.str());
  return ret;
}

WGL_API BOOL WINAPI wglShareLists(HGLRC, HGLRC){
  assert(false && "Implement me");
  return 1;
}

WGL_API BOOL WINAPI wglSwapBuffers(HDC hdc){
  std::ostringstream out;
  int num = gData->hdcs[hdc];
  out << WGLSWAPBUFFERS << " " << num;
  BOOL ret = (BOOL)wgl_interpret(out.str());
  return ret;
}
