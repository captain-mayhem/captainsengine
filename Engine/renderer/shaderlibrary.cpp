#include "shaderlibrary.h"

#include <cstring>

#include "builtinshaders.h"
#include "renderdevice.h"
#include "../io/Tracing.h"
#include "../platform/assets.h"

using namespace CGE::Gfx;

TR_CHANNEL(CGE_ShaderLibrary);

namespace{

const char* ENGINE_PREFIX = "engine/";
//! includes nested deeper are taken as a loop
const int MAX_INCLUDE_DEPTH = 16;

//! one file, built-in or asset, without resolving includes
bool loadFile(const std::string& path, std::string& text){
  if (path.compare(0, strlen(ENGINE_PREFIX), ENGINE_PREFIX) == 0){
    for (int i = 0; i < Builtin::fileCount; i++){
      if (path == Builtin::files[i].path){
        text = Builtin::files[i].source;
        return true;
      }
    }
    return false;
  }
  return CGE::Assets::loadText(path, text);
}

//! the path in an #include "path" line, or empty
std::string includePath(const std::string& line){
  size_t pos = line.find_first_not_of(" \t");
  if (pos == std::string::npos || line.compare(pos, 8, "#include") != 0)
    return "";
  size_t start = line.find('"', pos+8);
  size_t end = start == std::string::npos ? start : line.find('"', start+1);
  if (end == std::string::npos)
    return "";
  return line.substr(start+1, end-start-1);
}

bool resolve(const std::string& path, std::string& source, int depth){
  TR_USE(CGE_ShaderLibrary);
  if (depth > MAX_INCLUDE_DEPTH){
    TR_ERROR("Shader includes nested too deeply at %s", path.c_str());
    return false;
  }
  std::string text;
  if (!loadFile(path, text)){
    TR_ERROR("Shader file %s not found", path.c_str());
    return false;
  }
  size_t pos = 0;
  while (pos < text.size()){
    size_t end = text.find('\n', pos);
    if (end == std::string::npos)
      end = text.size();
    std::string line = text.substr(pos, end-pos);
    std::string include = includePath(line);
    if (include.empty())
      source += line + "\n";
    else if (!resolve(include, source, depth+1))
      return false;
    pos = end+1;
  }
  return true;
}

}

bool ShaderLibrary::loadSource(const std::string& path, std::string& source){
  source.clear();
  return resolve(path, source, 0);
}

bool ShaderLibrary::loadDesc(const std::string& vertexPath, const std::string& fragmentPath, ShaderDesc& desc){
  if (!loadSource(vertexPath, desc.vertexSource) || !loadSource(fragmentPath, desc.fragmentSource))
    return false;
  desc.uniformBlocks = {{"CGE_Frame", SLOT_FRAME}, {"CGE_Draw", SLOT_DRAW}, {"CGE_Material", SLOT_MATERIAL}, {"CGE_Lights", SLOT_LIGHTS}};
  desc.textures = {{"cge_texture", SLOT_TEXTURE}};
  if (desc.name.empty())
    desc.name = vertexPath;
  return true;
}

ShaderDesc ShaderLibrary::getBuiltinDesc(BuiltinShader shader){
  ShaderDesc desc;
  switch (shader){
    case BuiltinShader::Color:
      desc.name = "engine/color";
      break;
    case BuiltinShader::Textured:
      desc.name = "engine/textured";
      break;
    case BuiltinShader::Lit:
      desc.name = "engine/lit";
      break;
  }
  //the built-in files are part of the engine library, so this can't fail
  loadDesc(desc.name+".vert", desc.name+".frag", desc);
  return desc;
}

std::unique_ptr<Shader> ShaderLibrary::createBuiltin(RenderDevice& device, BuiltinShader shader){
  return device.createShader(getBuiltinDesc(shader));
}
