#include <string>
#include <sstream>
#include <vector>

#ifdef WIN32
#include <Windows.h>
#endif
#include <GL/gl.h>

#include "util_run.h"
#include "funcids.h"
#include "ogl_run.h"

typedef int   (WINAPI *PFNWGLCHOOSEPIXELFORMAT)(HDC,const PIXELFORMATDESCRIPTOR*);
typedef HGLRC (WINAPI *PFNWGLCREATECONTEXT)(HDC);
typedef BOOL  (WINAPI *PFNWGLDELETECONTEXT)(HGLRC);
typedef int   (WINAPI *PFNWGLDESCRIBEPIXELFORMAT)(HDC hdc, int iPixelFormat, unsigned nBytes, PIXELFORMATDESCRIPTOR *ppfd);
typedef int   (WINAPI *PFNWGLGETPIXELFORMAT)(HDC);
typedef BOOL  (WINAPI *PFNWGLMAKECURRENT)(HDC, HGLRC);
typedef BOOL  (WINAPI *PFNWGLSETPIXELFORMAT)(HDC, int, const PIXELFORMATDESCRIPTOR*);
typedef BOOL  (WINAPI *PFNWGLSHARELISTS)(HGLRC, HGLRC);
typedef BOOL  (WINAPI *PFNWGLSWAPBUFFERS)(HDC);

PFNWGLCHOOSEPIXELFORMAT choosePixelFormat = NULL;
PFNWGLCREATECONTEXT createContext = NULL;
PFNWGLDELETECONTEXT deleteContext = NULL;
PFNWGLDESCRIBEPIXELFORMAT describePixelFormat = NULL;
PFNWGLGETPIXELFORMAT getPixelFormat = NULL;
PFNWGLMAKECURRENT makeCurrent = NULL;
PFNWGLSETPIXELFORMAT setPixelFormat = NULL;
PFNWGLSHARELISTS shareLists = NULL;
PFNWGLSWAPBUFFERS swapBuffers = NULL;

static void cleanup();

struct WGLRunData{
  WGLRunData(){
    atexit(cleanup);
  }
  std::vector<HDC> mHDCs;
  std::vector<HGLRC> mHGLRCs;
};

static WGLRunData* gData = NULL;

void cleanup(){
  delete gData;
}

IMPEXP void wgl_init(){
  if (createContext != NULL)
    return;
  gData = new WGLRunData();
  void* oglhandle = loadLibrary("opengl32.dll");
  choosePixelFormat = (PFNWGLCHOOSEPIXELFORMAT)getSymbol(oglhandle, "wglChoosePixelFormat");
  createContext = (PFNWGLCREATECONTEXT)getSymbol(oglhandle, "wglCreateContext");
  deleteContext = (PFNWGLDELETECONTEXT)getSymbol(oglhandle, "wglDeleteContext");
  describePixelFormat = (PFNWGLDESCRIBEPIXELFORMAT)getSymbol(oglhandle, "wglDescribePixelFormat");
  getPixelFormat = (PFNWGLGETPIXELFORMAT)getSymbol(oglhandle, "wglGetPixelFormat");
  makeCurrent = (PFNWGLMAKECURRENT)getSymbol(oglhandle, "wglMakeCurrent");
  setPixelFormat = (PFNWGLSETPIXELFORMAT)getSymbol(oglhandle, "wglSetPixelFormat");
  shareLists = (PFNWGLSHARELISTS)getSymbol(oglhandle, "wglShareLists");
  swapBuffers = (PFNWGLSWAPBUFFERS)getSymbol(oglhandle, "wglSwapBuffers");
  gl_init(oglhandle);
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
        int ret = choosePixelFormat(gData->mHDCs[hdcidx], &pfd);
        return (void*)ret;
      }
    case WGLCREATECONTEXT:
      {
        int hdcidx;
        in >> hdcidx;
        HGLRC rc = createContext(gData->mHDCs[hdcidx]);
        gData->mHGLRCs.push_back(rc);
        return rc;
        break;
      }
    case WGLDELETECONTEXT:
      {
        int hglrcidx; in >> hglrcidx;
        BOOL ret = deleteContext(gData->mHGLRCs[hglrcidx]);
        return (void*)ret;
      }
    case WGLDESCRIBEPIXELFORMAT:
      {
        int hdcidx; in >> hdcidx;
        int pixelformat; in >> pixelformat;
        unsigned nBytes; in >> nBytes;
        void* pfd; in >> pfd;
        int ret = describePixelFormat(gData->mHDCs[hdcidx], pixelformat, nBytes, (PIXELFORMATDESCRIPTOR*)pfd);
        return (void*)ret;
      }
    case WGLGETPIXELFORMAT:
      {
        int hdcidx; in >> hdcidx;
        int ret = getPixelFormat(gData->mHDCs[hdcidx]);
        return (void*)ret;
      }
    case WGLMAKECURRENT:
      {
        int hdcidx; in >> hdcidx;
        int hglrcidx; in >> hglrcidx;
        BOOL ret = makeCurrent(gData->mHDCs[hdcidx], gData->mHGLRCs[hglrcidx]);
        return (void*)ret;
      }
    case WGLSETPIXELFORMAT:
      {
        int hdcidx; in >> hdcidx;
        int format; in >> format;
        PIXELFORMATDESCRIPTOR pfd;
        in >> pfd.nSize >> pfd.nVersion >> pfd.dwFlags >> pfd.iPixelType >> pfd.cColorBits >> pfd.cRedBits >> pfd.cRedShift >> pfd.cGreenBits >> pfd.cGreenShift >> pfd.cBlueBits >> pfd.cBlueShift >> pfd.cAlphaBits >> pfd.cAlphaShift >> pfd.cAccumBits >> pfd.cAccumRedBits >> pfd.cAccumGreenBits >> pfd.cAccumBlueBits >> pfd.cAccumAlphaBits >> pfd.cDepthBits >> pfd.cStencilBits >> pfd.cAuxBuffers >> pfd.iLayerType >> pfd.bReserved >> pfd.dwLayerMask >> pfd.dwVisibleMask >> pfd.dwDamageMask;
        BOOL ret = setPixelFormat(gData->mHDCs[hdcidx], format, &pfd);
        return (void*)ret;
      }
    case WGLSWAPBUFFERS:
      {
        int hdcidx; in >> hdcidx;
        BOOL ret = swapBuffers(gData->mHDCs[hdcidx]);
        return (void*)ret;
      }
    default:
      break;
  }
  return NULL;
}

IMPEXP void wgl_add_DC(HDC hdc){
  gData->mHDCs.push_back(hdc);
}