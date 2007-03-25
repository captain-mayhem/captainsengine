#ifndef GRAPHIC_H
#define GRAPHIC_H

#include <vector>

namespace MeshGeo{
class Mesh;
}

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

