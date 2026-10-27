// Dear ImGui's vertices (ImDrawVert): position in pixels, texture
// coordinates and colour. cge_projection maps ImGui's display to clip space.
#include "engine/common.glsl"

layout(location = 0) in vec2 a_position;
layout(location = 2) in vec4 a_color;
layout(location = 3) in vec2 a_texcoord;

out vec4 v_color;
out vec2 v_texcoord;

void main(){
  v_color = a_color;
  v_texcoord = a_texcoord;
  gl_Position = cge_projection*vec4(a_position, 0.0, 1.0);
}
