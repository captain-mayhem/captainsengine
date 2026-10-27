// multiplies with the pixels towards the centre
uniform sampler2D tex;

in vec2 v_texcoord;
in vec2 v_offset;

out vec4 o_color;

void main(){
  vec4 color = texture(tex, v_texcoord);
  color.rgb = color.rgb*texture(tex, v_texcoord-v_offset).rgb;
  color.rgb = color.rgb*texture(tex, v_texcoord-v_offset*vec2(2.0)).rgb;
  o_color = vec4(color.rgb, 1.0);
}
