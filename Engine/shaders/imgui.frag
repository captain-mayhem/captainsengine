// texture slot 0 (Gfx::SLOT_TEXTURE): ImGui's font atlas or the draw's texture
uniform sampler2D cge_texture;

in vec4 v_color;
in vec2 v_texcoord;

out vec4 o_color;

void main(){
  o_color = v_color*texture(cge_texture, v_texcoord);
}
