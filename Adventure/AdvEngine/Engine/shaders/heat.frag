// shifts each line sideways by the red channel of the one pixel wide
// blendtex (heat, underwater)
#include "adventure/effect.glsl"

uniform sampler2D tex;
uniform sampler2D blendtex;

in vec2 v_texcoord;

out vec4 o_color;

void main(){
  float xoffset = texture(blendtex, vec2(0.5, v_texcoord.t)).r*2.0-1.0;
  float xcoord = clamp(v_texcoord.s+xoffset*pixel_offset.x, 0.0, tex_scale.x-pixel_offset.x/2.0);
  vec4 color = texture(tex, vec2(xcoord, v_texcoord.t));
  o_color = vec4(color.rgb, 1.0);
}
