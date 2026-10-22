// darkens every third line by strength
#include "adventure/effect.glsl"

uniform sampler2D tex;

in vec2 v_texcoord;

out vec4 o_color;

void main(){
  vec4 color = texture(tex, v_texcoord);
  float rem = mod(v_texcoord.y, 3.0*pixel_offset.y);
  if (rem < pixel_offset.y)
    color *= strength;
  o_color = vec4(color.rgb, 1.0);
}
