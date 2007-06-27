#ifndef GRAPHIC_H
#define GRAPHIC_H

#include <vector>
#include "math/matrix.h"
#include "system/engine.h"
#include "renderer/scene.h"

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
  //! add a model to the editor
  void addModel(unsigned int idx);
  //! add a texture to the editor
  void addTexture(const std::string& filename);
  //! set transformation of a mesh
  void setTrafo(const int id, const Math::Matrix& mat);
  //! set translation of the camera
  void setCamTrafo(const Math::Matrix& mat) {camTrafo_ = mat;}
  //! set transformation of the camera
  void multCamTrafo(const Math::Matrix& mat) {camTrafo_ = mat * camTrafo_;}
  //! set transformation of the camera
  void setCamRotation(const Math::Matrix& mat) {camRot_ = mat;}
  //! get the scene
  Graphics::Scene& getScene() {return scene_;}
  //! set the current model
  void setCurrModel(MeshGeo::Model* mdl) {currModel_ = mdl;}
  //! get the current model
  MeshGeo::Model* getCurrModel() const {return currModel_;}
  //! get view matrix
  Math::Matrix getViewMat() {return camTrafo_ * camRot_;} 
protected:
  //! Constructor
  Graphic();
  //! the render method
  void render_();
  //! the instance
  static Graphic* gra_;
  //! the scene
  //std::vector<MeshGeo::Mesh*> meshes_;
  Graphics::Scene scene_;
  //! the camera transformation
  Math::Matrix camTrafo_;
  //! the camera rotation
  Math::Matrix camRot_;
  //! the current model
  MeshGeo::Model* currModel_;
};

#endif

