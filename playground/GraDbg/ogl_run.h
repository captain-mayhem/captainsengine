#ifndef OGL_RUN_H
#define OGL_RUN_H

#include "util_run.h"

IMPEXP void gl_init(void* lib);
IMPEXP void* gl_interpret(const std::string& cmd);

IMPEXP void gl_add_texturedata(const void* data);
IMPEXP void gl_add_vertexdata(const void* data);
IMPEXP void gl_add_texcoorddata(const void* data);

#endif
