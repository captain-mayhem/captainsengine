// SpriteBatch with distance fields (SpriteBatch::setDistanceField): the
// texture's alpha is the distance to a shape's edge, more inside, and the
// shape is filled with the sprite's colour up to the edge in cge_color.x,
// smoothed over about a pixel
#include "engine/common.glsl"

// texture slot 0 (Gfx::SLOT_TEXTURE)
uniform sampler2D cge_texture;

in vec4 v_color;
in vec2 v_texcoord;

out vec4 o_color;

void main(){
  float distance = texture(cge_texture, v_texcoord).a;
  // how much the distance changes over a pixel on the screen
  float smoothing = max(fwidth(distance)*0.5, 0.001);
  float coverage = smoothstep(cge_color.x - smoothing, cge_color.x + smoothing, distance);
  o_color = vec4(v_color.rgb, v_color.a*coverage);
}
