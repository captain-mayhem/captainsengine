#include "../lib/api.h"

int main(int argc, char** argv){
  VRSurface surface(200, 200);
  surface.clear(255, 255, 0, 255);
  Triangle tri(50, 10, 5, 70, 60, 80);
  tri.raster(&surface);
  Triangle tri2(100, 120, 50, 190, 150, 190);
  tri2.raster(&surface);
  surface.debugWrite("\\tmp\\test");
}
