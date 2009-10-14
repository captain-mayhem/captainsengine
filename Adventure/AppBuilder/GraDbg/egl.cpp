#include <EGL/egl.h>
#include <sstream>
#include <map>
#include "funcids.h"
#include "egl_run.h"

EGLAPI EGLBoolean EGLAPIENTRY eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config){

}

EGLAPI EGLContext EGLAPIENTRY eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list){

}

EGLAPI EGLSurface EGLAPIENTRY eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list){

}

EGLAPI EGLBoolean EGLAPIENTRY eglDestroyContext(EGLDisplay dpy, EGLContext ctx){

}

EGLAPI EGLBoolean EGLAPIENTRY eglDestroySurface(EGLDisplay dpy, EGLSurface surface){

}

EGLAPI EGLBoolean EGLAPIENTRY eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value){

}

EGLAPI EGLDisplay EGLAPIENTRY eglGetDisplay(EGLNativeDisplayType display_id){
  egl_init();
  std::ostringstream out;
  out << EGLGETDISPLAY << " " << display_id;
  EGLDisplay ret = egl_interpret(out.str());
  return ret;
}

EGLAPI EGLint EGLAPIENTRY eglGetError(void){
  std::ostringstream out;
  out << EGLGETERROR;
  return (EGLint)egl_interpret(out.str());
}

EGLAPI EGLBoolean EGLAPIENTRY eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor){
  std::ostringstream out;
  out << EGLINITIALIZE << " " << dpy << " " << major << " " << minor;
  return (EGLBoolean)egl_interpret(out.str());
}

EGLAPI EGLBoolean EGLAPIENTRY eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx){

}

EGLAPI EGLBoolean EGLAPIENTRY eglQuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value){

}

EGLAPI EGLBoolean EGLAPIENTRY eglSwapBuffers(EGLDisplay dpy, EGLSurface surface){

}

EGLAPI EGLBoolean EGLAPIENTRY eglTerminate(EGLDisplay dpy){

}
