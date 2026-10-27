// a shader asset that includes a built-in file (test_gldevice.cpp)
#include "engine/common.glsl"

layout(location = 0) in vec3 a_test;

void main(){
  gl_Position = cge_projection*vec4(a_test, 1.0);
}
