#include <GLES3/gl3.h>
#undef GL_APICALL
#define GL_APICALL
#include "../lib/api.h"

GLuint loadShader(VRShader* vrs){
  GLuint prog = GL(CreateProgram)();
  GLuint shader[2];
  shader[0] = GL(CreateShader)(GL_VERTEX_SHADER);
  shader[1] = GL(CreateShader)(GL_FRAGMENT_SHADER);
  GL(ShaderBinary)(2, shader, VR_SHADER_BINARY_FORMAT, vrs, sizeof(vrs));
  GL(AttachShader)(prog, shader[0]);
  GL(AttachShader)(prog, shader[1]);
  GL(DeleteShader)(shader[0]);
  GL(DeleteShader)(shader[1]);
  GL(LinkProgram)(prog);
  return prog;
}

int main(int argc, char** argv){
  VRSurface surface(200, 200);
  VRState context;
  context.setSurface(&surface);

  GL(ClearColor)(1.0f, 1.0f, 0.0f, 1.0f);
  GL(Clear)(GL_COLOR_BUFFER_BIT);

  float coord[] = {
    50, 10, 5, 70, 60, 80,
    100, 120, 50, 190, 150, 190
  };
  float color[] = {
    1, 0, 0, 1,
    0, 1, 0, 1,
    0, 0, 1, 1,

    1, 0, 0, 1,
    0, 1, 0, 1,
    0, 0, 1, 1
  };

  GouraudShader shader;
  FlatShader shader2;
  GLuint gouraud = loadShader(&shader);
  GLuint flat = loadShader(&shader2);

  GLuint program = gouraud;
  //GLuint program = flat;
  
  GL(UseProgram)(program);

  int vpos = GL(GetAttribLocation)(program, "position");
  int vcol = GL(GetAttribLocation)(program, "color");

  GL(EnableVertexAttribArray)(vpos);
  GL(EnableVertexAttribArray)(vcol);
  GL(VertexAttribPointer)(vpos, 2, GL_FLOAT, GL_FALSE, 0, coord);
  GL(VertexAttribPointer)(vcol, 4, GL_FLOAT, GL_FALSE, 0, color);

  for (int i = 0; i < 10000; ++i){
    GL(DrawArrays)(GL_TRIANGLES, 0, 6);
  }

  surface.debugWrite("\\tmp\\test");
}
