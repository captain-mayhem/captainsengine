#include "engine/lighting.glsl"

in vec3 v_position;
in vec3 v_normal;
in vec2 v_texcoord;
#ifdef CGE_VERTEX_COLOR
in vec4 v_color;
#endif

out vec4 o_color;

void main(){
  vec4 color = cge_surfaceColor(v_texcoord);
#ifdef CGE_VERTEX_COLOR
  color *= v_color;
#endif
  color = cge_alphaMode(color);
  o_color = cge_shade(color, cge_surfaceNormal(v_normal), v_position);
}
