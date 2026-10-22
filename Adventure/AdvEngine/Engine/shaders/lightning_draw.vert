// the lines of the lightnings, with positions from 0 to 1 over the target
layout(location = 0) in vec3 a_position;
layout(location = 2) in vec4 a_color;

out vec4 v_color;

void main(){
  v_color = a_color;
  gl_Position = vec4(a_position.xy*2.0-1.0, 0.0, 1.0);
}
