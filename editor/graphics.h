#ifndef GRAPHIC_H
#define GRAPHIC_H

#include <vector>

namespace MeshGeo{
class Mesh;
}

class Graphic{
public:
  ~Graphic();
  static void init();
  static void render();
  inline static Graphic* instance() {return gra_;}
  //! add a mesh to the editor
  void addMesh(std::string filename);
protected:
  //! Constructor
  Graphic();
  //! the render method
  void render_();
  //! the instance
  static Graphic* gra_;
  //! the meshes
  std::vector<MeshGeo::Mesh*> meshes_;
};

#endif

