#ifndef CGE_IMGUI_CONFIG_H
#define CGE_IMGUI_CONFIG_H

// The engine's Dear ImGui configuration, included by imgui.h through
// IMGUI_USER_CONFIG (set by the engine's CMake target for the engine and the
// applications). ImGui is compiled into the engine library and exported from
// it, so that the engine and the application share one ImGui context.

#include <cge_export.h>

#define IMGUI_API CGE_API

// 32-bit ImWchar: typed characters and fonts beyond the BMP (U+10000 and up)
#define IMGUI_USE_WCHAR32

#endif
