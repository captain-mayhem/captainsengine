#include "../lib/api.h"

int main(int argc, char** argv){
  VRSurface surface(200, 200);
  VRState context;
  context.setSurface(&surface);
  surface.clear(255, 255, 0, 255);
  float coord1[] = {50, 10, 5, 70, 60, 80};
  float color[] = {
    1, 0, 0, 1,
    0, 1, 0, 1,
    0, 0, 1, 1
  };
  Triangle tri(coord1);
  tri.setVarying(0, color);
  float coord2[] = {100, 120, 50, 190, 150, 190};
  Triangle tri2(coord2);
  tri2.setVarying(0, color);
  //context.enableVertexAttribArray(0, true);
  tri.raster(&context, 4);
  tri2.raster(&context, 4);
  tri2.setVarying(0, color);
  surface.debugWrite("\\tmp\\test");
}
