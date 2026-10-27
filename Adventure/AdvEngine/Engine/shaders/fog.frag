// mixes in a colour (fog: grey, flash: white)
#include "adventure/effect.glsl"

uniform sampler2D tex;

in vec2 v_texcoord;

out vec4 o_color;

void main(){
  vec4 color = texture(tex, v_texcoord);
  color = mix(color, blendcol, strength);
  o_color = vec4(color.rgb, 1.0);
}
