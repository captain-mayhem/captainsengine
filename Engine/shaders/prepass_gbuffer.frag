// Light prepass, step 1: the G-buffer of the opaque surfaces (with lit.vert).
#include "engine/lighting.glsl"

in vec3 v_position;
in vec3 v_normal;
in vec2 v_texcoord;

// the view-space normal (0..1) and the shininess/255
layout(location = 0) out vec4 o_normal;
// the surface colour; alpha 0 where nothing was drawn
layout(location = 1) out vec4 o_diffuse;
layout(location = 2) out vec4 o_specular;

void main(){
  o_normal = vec4(normalize(v_normal)*0.5 + 0.5, clamp(cge_specular.w/255.0, 0.0, 1.0));
  o_diffuse = cge_surfaceColor(v_texcoord);
  o_specular = vec4(cge_specular.rgb, 1.0);
}
