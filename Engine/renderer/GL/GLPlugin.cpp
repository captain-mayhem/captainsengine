// Entry points of the OpenGL backend as a plugin library (CGE_GL.dll /
// libCGE_GL.so), which RenderDevice::create loads. Not compiled where the
// backend is built into the engine (CGE_GL_BUILTIN).

#include "GLDevice.h"
#include "../renderbackend.h"

#ifdef WIN32
#define CGE_GL_EXPORT extern "C" __declspec(dllexport)
#else
#define CGE_GL_EXPORT extern "C" __attribute__((visibility("default")))
#endif

CGE_GL_EXPORT int cgeRenderBackendVersion(){
  return CGE_RENDER_BACKEND_VERSION;
}

CGE_GL_EXPORT CGE::Gfx::RenderDevice* cgeCreateRenderDevice(){
  return CGE::Gfx::createGLRenderDevice();
}
