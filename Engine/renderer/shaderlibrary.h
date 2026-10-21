#ifndef CGE_SHADER_LIBRARY_H
#define CGE_SHADER_LIBRARY_H

#include <cge_export.h>

#include <memory>
#include <string>

#include "gfxtypes.h"
#include "../math/matrix.h"

namespace CGE{
namespace Gfx{

class RenderDevice;
class Shader;

// The conventions of the built-in shaders (Engine/shaders/). Shaders of the
// projects can use them too by including "engine/common.glsl".

//! vertex attribute locations
enum BuiltinAttribute{
  ATTR_POSITION = 0,
  ATTR_NORMAL = 1,
  ATTR_COLOR = 2,
  ATTR_TEXCOORD = 3,
};

//! uniform buffer slot of the CGE_Frame block (FrameUniforms)
const int SLOT_FRAME = 0;
//! uniform buffer slot of the CGE_Draw block (DrawUniforms)
const int SLOT_DRAW = 1;
//! uniform buffer slot of the CGE_Material block (MaterialUniforms)
const int SLOT_MATERIAL = 2;
//! uniform buffer slot of the CGE_Lights block (LightUniforms)
const int SLOT_LIGHTS = 3;
//! the lights a lit draw takes into account
const int MAX_LIGHTS = 8;
//! texture slot of the cge_texture sampler
const int SLOT_TEXTURE = 0;

//! the CGE_Frame uniform block (std140)
struct FrameUniforms{
  Matrix projection = Matrix(Matrix::Identity);
  Matrix view = Matrix(Matrix::Identity);
  //! x: seconds since the start
  float time[4] = {0, 0, 0, 0};
};
static_assert(sizeof(FrameUniforms) == 144, "FrameUniforms must match CGE_Frame");

//! the CGE_Draw uniform block (std140)
struct DrawUniforms{
  Matrix model = Matrix(Matrix::Identity);
  //! multiplied with the vertex colour
  float color[4] = {1, 1, 1, 1};
  //! the inverse transpose of view*model, for normals in view space
  Matrix normalMatrix = Matrix(Matrix::Identity);
};
static_assert(sizeof(DrawUniforms) == 144, "DrawUniforms must match CGE_Draw");

//! the CGE_Material block (std140) of lit shaders (lighting.glsl)
struct MaterialUniforms{
  //! rgb, a: the opacity
  float diffuse[4] = {1, 1, 1, 1};
  float ambient[4] = {0, 0, 0, 1};
  //! rgb, w: the shininess (specular exponent)
  float specular[4] = {0, 0, 0, 10};
  //! x: 1 if the surface is textured
  int32_t flags[4] = {0, 0, 0, 0};
};
static_assert(sizeof(MaterialUniforms) == 64, "MaterialUniforms must match CGE_Material");

//! the CGE_Lights block (std140) of lit shaders (lighting.glsl), in view space
struct LightUniforms{
  //! x: the number of lights; without lights, surfaces are drawn unlit
  int32_t count[4] = {0, 0, 0, 0};
  //! w = 0: a directional light shining from the direction xyz
  float position[MAX_LIGHTS][4] = {};
  //! xyz: the spot direction, w: the cutoff angle in radians
  float direction[MAX_LIGHTS][4] = {};
  float color[MAX_LIGHTS][4] = {};
  //! x: the quadratic attenuation
  float attenuation[MAX_LIGHTS][4] = {};
};
static_assert(sizeof(LightUniforms) == 16+4*MAX_LIGHTS*16, "LightUniforms must match CGE_Lights");

enum class BuiltinShader{
  //! position and colour
  Color,
  //! position, colour and texture coordinates, with the texture in SLOT_TEXTURE
  Textured,
  //! position, normal and texture coordinates, lit with the material and lights
  /*! Unlit (the material's diffuse colour) without lights. The texture in
      SLOT_TEXTURE is used if the material's flags say so. */
  Lit,
};

//! Shader sources and the built-in shaders
/*! Shader files are GLSL in the common subset of GLSL 3.30 and GLSL ES 3.00,
    without a #version line (the backend adds it). They can include other
    files with #include "path". Paths starting with "engine/" name the
    engine's built-in files from Engine/shaders/, which are compiled into the
    engine library; other paths are assets (see Assets). */
class CGE_API ShaderLibrary{
public:
  //! the source of a shader file with its #include lines resolved
  static bool loadSource(const std::string& path, std::string& source);
  //! a shader description from a vertex and a fragment shader file
  /*! The description binds the engine's blocks and sampler (CGE_Frame,
      CGE_Draw, CGE_Material, CGE_Lights, cge_texture); add more bindings
      before creating the shader. */
  static bool loadDesc(const std::string& vertexPath, const std::string& fragmentPath, ShaderDesc& desc);
  //! the description of a built-in shader
  static ShaderDesc getBuiltinDesc(BuiltinShader shader);
  //! creates a built-in shader on a device; nullptr if that fails
  static std::unique_ptr<Shader> createBuiltin(RenderDevice& device, BuiltinShader shader);
};

}
}

#endif
