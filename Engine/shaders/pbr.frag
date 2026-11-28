// glTF's metallic-roughness material (pbr.glsl) with lit.vert. With
// CGE_VERTEX_COLOR defined (BuiltinShader::PbrVertexColor), the vertices'
// colours multiply the base colour.
#include "engine/pbr.glsl"

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
  CGE_PbrSurface surface;
  // the derivatives before a pixel is left out
  surface.normal = cge_pbrNormal(v_normal, v_position, v_texcoord);
  color = cge_alphaMode(color);
  surface.baseColor = cge_srgbToLinear(color.rgb);
  cge_pbrMaterial(surface, v_texcoord);
  o_color = vec4(cge_linearToSrgb(cge_pbrShade(surface, v_position)), color.a);
}
