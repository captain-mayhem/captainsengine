// mixes in coloured noise
#include "adventure/effect.glsl"

uniform sampler2D tex;
uniform sampler2D blendtex;

in vec2 v_texcoord;

out vec4 o_color;

void main(){
  vec4 color = texture(tex, v_texcoord);
  vec4 blendcol = vec4(1.0);
  blendcol.rgb = texture(blendtex, v_texcoord).rgb;
  color = mix(color, blendcol, strength);
  o_color = vec4(color.rgb, 1.0);
}
