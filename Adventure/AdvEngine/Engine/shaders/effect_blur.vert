// the quad over the target, with the direction from the centre (twice the
// pixel size at the edges) for the bloom effects
#include "adventure/effect.glsl"

layout(location = 0) in vec3 a_position;

out vec2 v_texcoord;
out vec2 v_offset;

void main(){
  v_texcoord = a_position.xy*tex_scale;
  v_offset = (a_position.xy*2.0-1.0)*pixel_offset*2.0;
  gl_Position = vec4(a_position.xy*2.0-1.0, 0.0, 1.0);
}
