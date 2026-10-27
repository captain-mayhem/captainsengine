// lays the drawn lightnings (blendtex) over the input
uniform sampler2D tex;
uniform sampler2D blendtex;

in vec2 v_texcoord;

out vec4 o_color;

void main(){
  vec4 color = texture(tex, v_texcoord);
  vec4 blendcol = texture(blendtex, v_texcoord);
  color = mix(color, blendcol, blendcol.a);
  o_color = vec4(color.rgb, 1.0);
}
