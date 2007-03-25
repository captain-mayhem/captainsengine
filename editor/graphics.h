#ifndef GRAPHIC_H
#define GRAPHIC_H

#include <vector>

class MeshGeo::Mesh;

class Graphic{
public:
  Graphic();
  ~Graphic();
  static void init();
  static void render();
protected:
  std::vector<MeshGeo::Mesh*> meshes_;
};

#endif

