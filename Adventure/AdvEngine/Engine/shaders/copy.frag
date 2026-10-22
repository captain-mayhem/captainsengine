// the input unchanged (drugged, zoom, and the frames of the motion blur)
uniform sampler2D tex;

in vec2 v_texcoord;

out vec4 o_color;

void main(){
  vec4 color = texture(tex, v_texcoord);
  o_color = vec4(color.rgb, 1.0);
}
