#include "engine/lighting.glsl"

in vec3 v_position;
in vec3 v_normal;
in vec2 v_texcoord;

out vec4 o_color;

void main(){
  o_color = cge_shade(cge_surfaceColor(v_texcoord), normalize(v_normal), v_position);
}
