// mixes the frame with the three taken before, the older ones weaker
uniform sampler2D tex;
uniform sampler2D motion1;
uniform sampler2D motion2;
uniform sampler2D motion3;

in vec2 v_texcoord;

out vec4 o_color;

void main(){
  vec4 color = vec4(1.0);
  vec4 blendcol = vec4(1.0);
  color.rgb = texture(motion3, v_texcoord).rgb;
  blendcol.rgb = texture(motion2, v_texcoord).rgb;
  color = mix(color, blendcol, 0.5);
  blendcol.rgb = texture(motion1, v_texcoord).rgb;
  color = mix(color, blendcol, 0.5);
  blendcol.rgb = texture(tex, v_texcoord).rgb;
  color = mix(color, blendcol, 0.5);
  o_color = vec4(color.rgb, 1.0);
}
