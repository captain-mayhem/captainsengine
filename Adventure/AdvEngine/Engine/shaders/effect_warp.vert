// the quad over the target with each corner moved by an offset (drugged, zoom)
#include "adventure/effect.glsl"

layout(location = 0) in vec3 a_position;
layout(location = 4) in vec2 a_offset;

out vec2 v_texcoord;

void main(){
  v_texcoord = a_position.xy*tex_scale;
  gl_Position = vec4(a_position.xy*2.0-1.0+a_offset, 0.0, 1.0);
}
