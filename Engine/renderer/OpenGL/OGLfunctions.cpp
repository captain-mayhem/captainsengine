#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif !defined(ANDROID)
#include <EGL/egl.h>
#endif

#include "OGLfunctions.h"

#if !defined(ANDROID)

namespace{

GLADapiproc getProcAddress(const char* name){
#ifdef WIN32
  //wglGetProcAddress knows only the functions after OpenGL 1.1
  PROC proc = wglGetProcAddress(name);
  if (proc == nullptr || proc == (PROC)1 || proc == (PROC)2 || proc == (PROC)3 || proc == (PROC)-1)
    proc = GetProcAddress(GetModuleHandleA("opengl32.dll"), name);
  return (GLADapiproc)proc;
#else
  //the GL backend creates an EGL context on Linux
  return (GLADapiproc)eglGetProcAddress(name);
#endif
}

}

bool CGE::loadGLFunctions(){
  return gladLoadGL(getProcAddress) != 0;
}

#else

bool CGE::loadGLFunctions(){
  return true;
}

#endif
