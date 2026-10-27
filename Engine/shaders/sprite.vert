// SpriteBatch (renderer/spritebatch.h): one instance per sprite, drawn as the
// unit quad. The sprites are already transformed (CPU side), so cge_model
// and cge_color aren't used.
#include "engine/common.glsl"

// the unit quad's corner, (0, 0) to (1, 1)
layout(location = 0) in vec2 a_corner;
// per instance: where the corners (1, 0) and (0, 1) go relative to (0, 0)
layout(location = 4) in vec4 a_axes;
// per instance: where the corner (0, 0) goes
layout(location = 5) in vec3 a_origin;
// per instance: the texture coordinates of the corners (0, 0) and (1, 1)
layout(location = 6) in vec4 a_texrect;
layout(location = 2) in vec4 a_color;

out vec4 v_color;
out vec2 v_texcoord;

void main(){
  vec2 pos = a_origin.xy + a_axes.xy*a_corner.x + a_axes.zw*a_corner.y;
  v_color = a_color;
  v_texcoord = mix(a_texrect.xy, a_texrect.zw, a_corner);
  gl_Position = cge_projection*cge_view*vec4(pos, a_origin.z, 1.0);
}
