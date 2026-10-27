// the quad over the target with the input's texture coordinates
#include "adventure/effect.glsl"

layout(location = 0) in vec3 a_position;

out vec2 v_texcoord;

void main(){
  v_texcoord = a_position.xy*tex_scale;
  gl_Position = vec4(a_position.xy*2.0-1.0, 0.0, 1.0);
}
