#ifndef CGE_BUILTIN_SHADERS_H
#define CGE_BUILTIN_SHADERS_H

// The engine's built-in shader files, generated from Engine/shaders/ by
// shaders/embed.cmake (internal to the engine).

#include "shaderlibrary.h"

namespace CGE{
namespace Gfx{
namespace Builtin{

//! the paths are "engine/" and the file name
extern const ShaderFile files[];
extern const int fileCount;

}
}
}

#endif
