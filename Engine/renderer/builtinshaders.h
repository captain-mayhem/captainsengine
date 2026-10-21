#ifndef CGE_BUILTIN_SHADERS_H
#define CGE_BUILTIN_SHADERS_H

// The engine's built-in shader files, generated from Engine/shaders/ by
// shaders/embed.cmake (internal to the engine).

namespace CGE{
namespace Gfx{
namespace Builtin{

struct ShaderFile{
  //! "engine/" and the file name
  const char* path;
  const char* source;
};
extern const ShaderFile files[];
extern const int fileCount;

}
}
}

#endif
