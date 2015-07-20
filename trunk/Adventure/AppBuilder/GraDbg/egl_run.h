#ifndef EGL_RUN_H
#define EGL_RUN_H

#include "util_run.h"

IMPEXP void egl_init();
IMPEXP void* egl_interpret(const std::string& cmd);


#endif
