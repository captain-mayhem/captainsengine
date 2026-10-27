// The Adventure's quads and shapes (Graphics.cpp): the engine's matrices and
// colour, with the texture coordinates moved into the used part of a texture.
#include "engine/common.glsl"

// uniform buffer slot 4 (Graphics::SLOT_ADVENTURE)
layout(std140) uniform ADV_Blit{
  // xy: the scale, zw: the offset of the texture coordinates
  vec4 adv_texTransform;
};

layout(location = 0) in vec3 a_position;
layout(location = 3) in vec2 a_texcoord;

out vec2 v_texcoord;
out vec4 v_color;

void main(){
  v_texcoord = a_texcoord*adv_texTransform.xy+adv_texTransform.zw;
  v_color = cge_color;
  gl_Position = cge_projection*cge_view*cge_model*vec4(a_position, 1.0);
}
