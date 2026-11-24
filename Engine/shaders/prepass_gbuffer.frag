// Light prepass, step 1: the G-buffer of the opaque surfaces (with lit.vert).
#include "engine/lighting.glsl"

in vec3 v_position;
in vec3 v_normal;
in vec2 v_texcoord;
#ifdef CGE_VERTEX_COLOR
in vec4 v_color;
#endif

// the view-space normal (0..1) and the shininess/255
layout(location = 0) out vec4 o_normal;
// the surface colour; alpha 0 where nothing was drawn
layout(location = 1) out vec4 o_diffuse;
layout(location = 2) out vec4 o_specular;

void main(){
  vec4 color = cge_surfaceColor(v_texcoord);
#ifdef CGE_VERTEX_COLOR
  color *= v_color;
#endif
  color = cge_alphaMode(color);
  o_normal = vec4(cge_surfaceNormal(v_normal)*0.5 + 0.5, clamp(cge_specular.w/255.0, 0.0, 1.0));
  o_diffuse = color;
  o_specular = vec4(cge_specular.rgb, 1.0);
}
