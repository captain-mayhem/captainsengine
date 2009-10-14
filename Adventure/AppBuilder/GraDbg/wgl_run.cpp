#include <string>
#include <sstream>
#include <vector>

#ifdef WIN32
#include <Windows.h>
#endif
#include <GL/gl.h>

#include "util_run.h"
#include "funcids.h"

typedef int   (WINAPI *PFNWGLCHOOSEPIXELFORMAT)(HDC,const PIXELFORMATDESCRIPTOR*);
typedef HGLRC (WINAPI *PFNWGLCREATECONTEXT)(HDC);
typedef BOOL  (WINAPI *PFNWGLDELETECONTEXT)(HGLRC);
typedef int   (WINAPI *PFNWGLDESCRIBEPIXELFORMAT)(HDC hdc, int iPixelFormat, unsigned nBytes, PIXELFORMATDESCRIPTOR *ppfd);
typedef int   (WINAPI *PFNWGLGETPIXELFORMAT)(HDC);
typedef BOOL  (WINAPI *PFNWGLMAKECURRENT)(HDC, HGLRC);
typedef BOOL  (WINAPI *PFNWGLSETPIXELFORMAT)(HDC, int, const PIXELFORMATDESCRIPTOR*);
typedef BOOL  (WINAPI *PFNWGLSHARELISTS)(HGLRC, HGLRC);

PFNWGLCHOOSEPIXELFORMAT choosePixelFormat = NULL;
PFNWGLCREATECONTEXT createContext = NULL;
PFNWGLDELETECONTEXT deleteContext = NULL;
PFNWGLDESCRIBEPIXELFORMAT describePixelFormat = NULL;
PFNWGLGETPIXELFORMAT getPixelFormat = NULL;
PFNWGLMAKECURRENT makeCurrent = NULL;
PFNWGLSETPIXELFORMAT setPixelFormat = NULL;
PFNWGLSHARELISTS shareLists = NULL;

std::vector<HDC> mHDCs;
//std::vector<HGLRC> mHGLRCs;

IMPEXP void wgl_init(){
  if (createContext != NULL)
    return;
  //mHDCs.push_back(NULL);
  void* oglhandle = loadLibrary("opengl32.dll");
  choosePixelFormat = (PFNWGLCHOOSEPIXELFORMAT)getSymbol(oglhandle, "wglChoosePixelFormat");
  createContext = (PFNWGLCREATECONTEXT)getSymbol(oglhandle, "wglCreateContext");
  deleteContext = (PFNWGLDELETECONTEXT)getSymbol(oglhandle, "wglDeleteContext");
  describePixelFormat = (PFNWGLDESCRIBEPIXELFORMAT)getSymbol(oglhandle, "wglDescribePixelFormat");
  getPixelFormat = (PFNWGLGETPIXELFORMAT)getSymbol(oglhandle, "wglGetPixelFormat");
  makeCurrent = (PFNWGLMAKECURRENT)getSymbol(oglhandle, "wglMakeCurrent");
  setPixelFormat = (PFNWGLSETPIXELFORMAT)getSymbol(oglhandle, "wglSetPixelFormat");
  shareLists = (PFNWGLSHARELISTS)getSymbol(oglhandle, "shareLists");
}

IMPEXP void* wgl_interpret(const std::string& cmd){
  int curr = 0;
  int opcode;
  std::istringstream in(cmd);
  in >> opcode;
  switch(opcode){
    case WGLCHOOSEPIXELFORMAT:
      {
        int hdcidx;
        in >> hdcidx;
        PIXELFORMATDESCRIPTOR pfd;
        in >> pfd.nSize >> pfd.nVersion >> pfd.dwFlags >> pfd.iPixelType >> pfd.cColorBits >> pfd.cRedBits >> pfd.cRedShift >> pfd.cGreenBits >> pfd.cGreenShift >> pfd.cBlueBits >> pfd.cBlueShift >> pfd.cAlphaBits >> pfd.cAlphaShift >> pfd.cAccumBits >> pfd.cAccumRedBits >> pfd.cAccumGreenBits >> pfd.cAccumBlueBits >> pfd.cAccumAlphaBits >> pfd.cDepthBits >> pfd.cStencilBits >> pfd.cAuxBuffers >> pfd.iLayerType >> pfd.bReserved >> pfd.dwLayerMask >> pfd.dwVisibleMask >> pfd.dwDamageMask;
        int ret = choosePixelFormat(mHDCs[hdcidx], &pfd);
        return (void*)ret;
      }
    case WGLCREATECONTEXT:
      {
        int hdcidx;
        in >> hdcidx;
        HGLRC rc = createContext(mHDCs[hdcidx]);
        //mHGLRCs.push_back(rc);
        return rc;
        break;
      }
    case WGLDESCRIBEPIXELFORMAT:
      {
        int hdcidx; in >> hdcidx;
        int pixelformat; in >> pixelformat;
        unsigned nBytes; in >> nBytes;
        void* pfd; in >> pfd;
        int ret = describePixelFormat(mHDCs[hdcidx], pixelformat, nBytes, (PIXELFORMATDESCRIPTOR*)pfd);
        return (void*)ret;
      }
    case WGLGETPIXELFORMAT:
      {
        int hdcidx; in >> hdcidx;
        int ret = getPixelFormat(mHDCs[hdcidx]);
        return (void*)ret;
      }
    case WGLMAKECURRENT:
      {
        int hdcidx; in >> hdcidx;
        int hglrcidx; in >> hglrcidx;
        //BOOL ret = makeCurrent(mHDCs[hdcidx], mHGLRCs[hglrcidx]);
        BOOL ret = 1;
        return (void*)ret;
      }
    case WGLSETPIXELFORMAT:
      {
        int hdcidx; in >> hdcidx;
        int format; in >> format;
        PIXELFORMATDESCRIPTOR pfd;
        in >> pfd.nSize >> pfd.nVersion >> pfd.dwFlags >> pfd.iPixelType >> pfd.cColorBits >> pfd.cRedBits >> pfd.cRedShift >> pfd.cGreenBits >> pfd.cGreenShift >> pfd.cBlueBits >> pfd.cBlueShift >> pfd.cAlphaBits >> pfd.cAlphaShift >> pfd.cAccumBits >> pfd.cAccumRedBits >> pfd.cAccumGreenBits >> pfd.cAccumBlueBits >> pfd.cAccumAlphaBits >> pfd.cDepthBits >> pfd.cStencilBits >> pfd.cAuxBuffers >> pfd.iLayerType >> pfd.bReserved >> pfd.dwLayerMask >> pfd.dwVisibleMask >> pfd.dwDamageMask;
        BOOL ret = setPixelFormat(mHDCs[hdcidx], format, &pfd);
        return (void*)ret;
      }
    default:
      break;
  }
  return 0;
}

IMPEXP void wgl_add_DC(HDC hdc){
  mHDCs.push_back(hdc);
}