// darkens with the pixels towards the centre
#include "adventure/effect.glsl"

uniform sampler2D tex;

in vec2 v_texcoord;
in vec2 v_offset;

out vec4 o_color;

void main(){
  vec4 color = texture(tex, v_texcoord);
  vec4 intensity = vec4(strength)/vec4(6.0);
  intensity.a = 0.0;
  color = color-texture(tex, v_texcoord-v_offset)*intensity;
  color = color-texture(tex, v_texcoord-v_offset*vec2(2.0))*intensity;
  color = color-texture(tex, v_texcoord-v_offset*vec2(3.0))*intensity;
  color = color-texture(tex, v_texcoord-v_offset*vec2(4.0))*intensity;
  color = color-texture(tex, v_texcoord-v_offset*vec2(5.0))*intensity;
  color = color-texture(tex, v_texcoord-v_offset*vec2(6.0))*intensity;
  o_color = vec4(color.rgb, 1.0);
}
