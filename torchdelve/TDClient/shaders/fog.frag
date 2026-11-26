// The 3D view's shader (with engine/lit.vert): the hero's torch and TDClient's
// dark fog.
// - The torch is the first light (a point light just above the eyes,
//   renderer.cpp). Its light wraps around the surfaces ("wrapped diffuse"):
//   the walls of a corridor, which it only grazes, still catch some of it.
// - A faint cool light everywhere, so nothing in reach is all black.
// - Exponential squared fog by the distance along the view axis, as the old
//   GL2 shader and the fixed-function GL_EXP2 fog had it.
#include "engine/lighting.glsl"

const float FOG_DENSITY = 0.035;
const vec4 FOG_COLOR = vec4(0.0, 0.0, 0.0, 1.0);
// how far the light wraps around (0: plain Lambert)
const float WRAP = 0.7;
const vec3 AMBIENT = vec3(0.05, 0.055, 0.07);

in vec3 v_position;
in vec3 v_normal;
in vec2 v_texcoord;

out vec4 o_color;

void main(){
  vec4 color = cge_alphaMode(cge_surfaceColor(v_texcoord));
  vec3 normal = cge_surfaceNormal(v_normal);
  vec3 light = AMBIENT;
  vec3 specular = vec3(0.0);
  if (cge_lightCount.x > 0){
    float att;
    vec3 lightvec = cge_lightVector(cge_lightPosition[0], cge_lightDirection[0], cge_lightAttenuation[0].x, v_position, att);
    float wrapped = max((dot(normal, lightvec) + WRAP)/(1.0 + WRAP), 0.0);
    light += cge_lightColor[0].rgb*wrapped*att;
    vec3 refl = reflect(-lightvec, normal);
    float spec = pow(max(dot(refl, normalize(-v_position)), 0.0), max(cge_specular.w, 1.0));
    specular = cge_lightColor[0].rgb*cge_specular.rgb*spec*att;
  }
  color = vec4(color.rgb*light + specular, color.a);
  float fog = 1.0 - clamp(exp(-pow(FOG_DENSITY*abs(v_position.z), 2.0)), 0.0, 1.0);
  o_color = mix(color, FOG_COLOR, fog);
}
