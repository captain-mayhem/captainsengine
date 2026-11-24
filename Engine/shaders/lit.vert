// Lit by the engine's lights with the engine's material (lighting.glsl), in
// view space. Also the vertex shader of the light-prepass G-buffer. With
// CGE_VERTEX_COLOR defined (BuiltinShader::LitVertexColor), the vertices
// have colours, which multiply the surface's.
#include "engine/common.glsl"

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
#ifdef CGE_VERTEX_COLOR
layout(location = 2) in vec4 a_color;
#endif
layout(location = 3) in vec2 a_texcoord;

out vec3 v_position;
out vec3 v_normal;
out vec2 v_texcoord;
#ifdef CGE_VERTEX_COLOR
out vec4 v_color;
#endif

void main(){
  vec4 position = cge_view*cge_model*vec4(a_position, 1.0);
  v_position = position.xyz;
  v_normal = (cge_normalMatrix*vec4(a_normal, 0.0)).xyz;
  v_texcoord = a_texcoord;
#ifdef CGE_VERTEX_COLOR
  v_color = a_color;
#endif
  gl_Position = cge_projection*position;
}
