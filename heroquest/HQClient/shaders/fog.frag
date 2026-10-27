// The engine's lit shader (with engine/lit.vert) with HQClient's dark fog:
// exponential squared fog by the distance along the view axis, as the old
// GL2 shader and the fixed-function GL_EXP2 fog had it.
#include "engine/lighting.glsl"

const float FOG_DENSITY = 0.035;
const vec4 FOG_COLOR = vec4(0.0, 0.0, 0.0, 1.0);

in vec3 v_position;
in vec3 v_normal;
in vec2 v_texcoord;

out vec4 o_color;

void main(){
  vec4 color = cge_shade(cge_surfaceColor(v_texcoord), normalize(v_normal), v_position);
  float fog = 1.0 - clamp(exp(-pow(FOG_DENSITY*abs(v_position.z), 2.0)), 0.0, 1.0);
  o_color = mix(color, FOG_COLOR, fog);
}
