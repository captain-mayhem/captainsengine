// Unlit, textured: the texture times the vertex colour times cge_color.
#include "engine/common.glsl"

layout(location = 0) in vec3 a_position;
layout(location = 2) in vec4 a_color;
layout(location = 3) in vec2 a_texcoord;

out vec4 v_color;
out vec2 v_texcoord;

void main(){
  v_color = a_color*cge_color;
  v_texcoord = a_texcoord;
  gl_Position = cge_projection*cge_view*cge_model*vec4(a_position, 1.0);
}
