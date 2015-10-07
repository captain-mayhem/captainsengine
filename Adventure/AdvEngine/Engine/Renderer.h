#ifndef ADV_RENDERER_H
#define ADV_RENDERER_H

#include <cstdlib>
#include <system/engine.h>
#include <renderer/renderer.h>
//#include <renderer/forms.h>
#include <renderer/GL2/GL2Shader.h>

//#define RENDER_TEGRA

#ifdef RENDER_TEGRA
#include <GLES2/gl2.h>
#define RENDER_GL2
#else
#ifdef WIN32
#include <windows.h>
#endif
//#include <GL/glew.h>
#define RENDER_GL2
#endif

#endif
