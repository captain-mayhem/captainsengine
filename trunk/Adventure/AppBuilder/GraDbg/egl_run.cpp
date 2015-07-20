#include "egl_run.h"
#include <EGL/egl.h>
#include <sstream>
#include "funcids.h"

typedef EGLDisplay (EGLAPIENTRY *PFNEGLGETDISPLAY)(EGLNativeDisplayType display_id);
typedef EGLint (EGLAPIENTRY *PFNEGLGETERROR)(void);

PFNEGLGETDISPLAY getDisplay = NULL;
PFNEGLGETERROR getError = NULL;

IMPEXP void egl_init(){
  if (getDisplay != NULL)
    return;
  void* eglhandle = loadLibrary("libEGL.so.1");
  getDisplay = (PFNEGLGETDISPLAY)getSymbol(eglhandle, "eglGetDisplay");
  getError = (PFNEGLGETERROR)getSymbol(eglhandle, "eglGetError");
}

IMPEXP void* egl_interpret(const std::string& cmd){
  int curr = 0;
  int opcode;
  std::istringstream in(cmd);
  in >> opcode;
  switch(opcode){
    case EGLGETDISPLAY:
        {
            EGLNativeDisplayType display; in >> display;
            EGLDisplay disp = getDisplay(display);
            return disp;
        }
    case EGLGETERROR:
        {
            return (void*)getError();
        }
  	/*
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
      }*/
    default:
      break;
  }
  return 0;
}