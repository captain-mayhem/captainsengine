// averages blocks of strength x strength pixels, which move with the camera
#include "adventure/effect.glsl"

uniform sampler2D tex;

in vec2 v_texcoord;

out vec4 o_color;

void main(){
  vec2 delta = strength*pixel_offset;
  vec2 basecoord = delta*floor(v_texcoord/delta);
  vec4 color = vec4(0.0);
  for (float i = -cam_offset.x; i < strength-cam_offset.x; ++i){
    for (float j = -cam_offset.y; j < strength-cam_offset.y; ++j){
      vec2 coord = basecoord+vec2(i*pixel_offset.x, j*pixel_offset.y);
      color += texture(tex, clamp(coord, pixel_offset, vec2(1.0)));
    }
  }
  color /= strength*strength;
  o_color = vec4(color.rgb, 1.0);
}
