// Unlit, coloured: the vertex colour times cge_color.
#include "engine/common.glsl"

layout(location = 0) in vec3 a_position;
layout(location = 2) in vec4 a_color;

out vec4 v_color;

void main(){
  v_color = a_color*cge_color;
  gl_Position = cge_projection*cge_view*cge_model*vec4(a_position, 1.0);
}
