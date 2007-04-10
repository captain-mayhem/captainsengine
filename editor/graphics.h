#ifndef GRAPHIC_H
#define GRAPHIC_H

#include <vector>
#include "math/matrix.h"

namespace MeshGeo{
class Mesh;
}

class Graphic{
public:
  ~Graphic();
  static void init();
  static void render();
  inline static Graphic* instance() {return gra_;}
  inline static void release() {SAFE_DELETE(gra_);}
  //! add a mesh to the editor
  void addMesh(std::string filename);
  //! set transformation of a mesh
  void setTrafo(const int id, const Math::Matrix& mat);
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

