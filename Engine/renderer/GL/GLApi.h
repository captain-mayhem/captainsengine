#ifndef CGE_GL_API_H
#define CGE_GL_API_H

// The OpenGL API of the GL backend: glad (OpenGL 3.3) on desktop, the
// OpenGL ES 3.0 headers on Android. Only the backend's own files include it.

#if defined(ANDROID)
#include <GLES3/gl3.h>
#define CGE_GLES 1
#else
#include <glad/gl.h>
#endif

#endif
