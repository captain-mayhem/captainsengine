// a shader compiled into cge_tests by cge_embed_shaders (test_gldevice.cpp)
#include "engine/common.glsl"

layout(location = 0) in vec3 a_embedded;

void main(){
  gl_Position = cge_projection*vec4(a_embedded, 1.0);
}
