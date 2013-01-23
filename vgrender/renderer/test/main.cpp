#include <GLES3/gl3.h>
#undef GL_APICALL
#define GL_APICALL
#include "../lib/api.h"

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
  context.setShader(&shader);
  GL(VertexAttribPointer)(0, 2, GL_FLOAT, GL_FALSE, 0, coord);
  GL(VertexAttribPointer)(1, 4, GL_FLOAT, GL_FALSE, 0, color);
  Triangle tri(coord);
  tri.setVarying(0, color);
  Triangle tri2(coord+2*3);
  tri2.setVarying(0, color);
  //context.enableVertexAttribArray(0, true);
  tri.raster(&context, 1);
  tri2.raster(&context, 1);
  surface.debugWrite("\\tmp\\test");
}
