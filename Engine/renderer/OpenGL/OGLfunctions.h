#ifndef CGE_OGL_FUNCTIONS_H
#define CGE_OGL_FUNCTIONS_H

// The GL header of the old renderers (OpenGL and GL2): glad on desktop,
// OpenGL ES 2.0 on Android. Internal to the engine; deleted with them.

#if defined(ANDROID)
#include <GLES2/gl2.h>
#else
#include <glad/gl.h>
#endif

namespace CGE{

//! loads the engine's copy of the GL functions from the current context
/*! The GL backend plugin has its own copy of glad, which it loads itself.
    Nothing to do on Android, which links OpenGL ES directly. */
bool loadGLFunctions();

}

#endif
