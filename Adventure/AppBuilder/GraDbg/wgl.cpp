#include "util.h"
#include <sstream>
#include <map>
#include "funcids.h"
#include "wgl_run.h"

#ifdef WIN32
#define WINGDIAPI __declspec(dllexport)
#endif
#define WGL_API WINGDIAPI
#define WINAPI
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

static int hdccount = 0;
std::map<HDC, int> hdcs;

WGL_API int WINAPI wglChoosePixelFormat(HDC hdc, const PIXELFORMATDESCRIPTOR *ppfd){
  wgl_init();
  wgl_add_DC(hdc);
  if (hdcs[hdc] == 0)
    hdcs[hdc] = ++hdccount;
  std::ostringstream out;
  int num = hdcs[hdc];
  out << WGLCHOOSEPIXELFORMAT << " " << num;
  out << " " << ppfd->nSize << " " << ppfd->nVersion << " " << ppfd->dwFlags << " " << ppfd->iPixelType << " " << ppfd->cColorBits << " " << ppfd->cRedBits << " " << ppfd->cRedShift << " " << ppfd->cGreenBits << " " << ppfd->cGreenShift << " " << ppfd->cBlueBits << " " << ppfd->cBlueShift << " " << ppfd->cAlphaBits << " " << ppfd->cAlphaShift << " " << ppfd->cAccumBits << " " << ppfd->cAccumRedBits << " " << ppfd->cAccumGreenBits << " " << ppfd->cAccumBlueBits << " " << ppfd->cAccumAlphaBits << " " << ppfd->cDepthBits << " " << ppfd->cStencilBits << " " << ppfd->cAuxBuffers << " " << ppfd->iLayerType << " " << ppfd->bReserved << " " << ppfd->dwLayerMask << " " << ppfd->dwVisibleMask << " " << ppfd->dwDamageMask;
  int ret = (int)wgl_interpret(out.str());
  return ret;
}

WGL_API HGLRC WINAPI wglCreateContext(HDC hdc){
  std::ostringstream out;
  int num = hdcs[hdc];
  out << WGLCREATECONTEXT << " " << num;
  return (HGLRC)wgl_interpret(out.str());
}

WGL_API BOOL WINAPI wglDeleteContext(HGLRC){
  return 1;
}

WINGDIAPI int WINAPI wglDescribePixelFormat(HDC hdc, int iPixelFormat, unsigned nBytes, PIXELFORMATDESCRIPTOR *ppfd){
  std::ostringstream out;
  int num = hdcs[hdc];
  out << WGLDESCRIBEPIXELFORMAT << " " << num << " " << iPixelFormat << " " << nBytes << " " << ppfd;
  int ret = (int)wgl_interpret(out.str());
  return ret;
}

WGL_API int WINAPI wglGetPixelFormat(HDC hdc){
  return 0;
}

WGL_API BOOL  WINAPI wglMakeCurrent(HDC, HGLRC){
  return 1;
}

WGL_API BOOL  WINAPI wglSetPixelFormat(HDC hdc, int format, const PIXELFORMATDESCRIPTOR * ppfd){
  return 1;
}

WGL_API BOOL WINAPI wglShareLists(HGLRC, HGLRC){
  return 1;
}

WGL_API BOOL WINAPI wglSwapBuffers(HDC){
  return 1;
}
