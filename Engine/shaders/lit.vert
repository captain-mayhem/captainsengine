// Lit by the engine's lights with the engine's material (lighting.glsl), in
// view space. Also the vertex shader of the light-prepass G-buffer.
#include "engine/common.glsl"

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 3) in vec2 a_texcoord;

out vec3 v_position;
out vec3 v_normal;
out vec2 v_texcoord;

void main(){
  vec4 position = cge_view*cge_model*vec4(a_position, 1.0);
  v_position = position.xyz;
  v_normal = (cge_normalMatrix*vec4(a_normal, 0.0)).xyz;
  v_texcoord = a_texcoord;
  gl_Position = cge_projection*position;
}
