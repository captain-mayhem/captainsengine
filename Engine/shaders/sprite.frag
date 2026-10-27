// SpriteBatch: the texture times the sprite's colour
// texture slot 0 (Gfx::SLOT_TEXTURE)
uniform sampler2D cge_texture;

in vec4 v_color;
in vec2 v_texcoord;

out vec4 o_color;

void main(){
  o_color = texture(cge_texture, v_texcoord)*v_color;
}
