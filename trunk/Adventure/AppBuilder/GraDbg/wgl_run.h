#ifndef WGL_RUN_H
#define WGL_RUN_H

#include "util_run.h"

#ifndef DECLARE_HANDLE
typedef void* HANDLE;
#define DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
DECLARE_HANDLE(HDC);
#endif

IMPEXP void wgl_init();
IMPEXP void* wgl_interpret(const std::string& cmd);

IMPEXP void wgl_add_DC(HDC hdc);


#endif
