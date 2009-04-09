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
  void setTrafo(const int id, const CGE::Matrix& mat);
  //! set translation of the camera
  void setCamTrafo(const CGE::Matrix& mat) {camTrafo_ = mat;}
  //! set transformation of the camera
  void multCamTrafo(const CGE::Matrix& mat) {camTrafo_ = mat * camTrafo_;}
  //! set transformation of the camera
  void setCamRotation(const CGE::Matrix& mat) {camRot_ = mat;}
  //! get the scene
  CGE::Scene& getScene() {return scene_;}
  //! set the current model
  void setCurrModel(MeshGeo::Model* mdl) {currModel_ = mdl;}
  //! get the current model
  MeshGeo::Model* getCurrModel() const {return currModel_;}
  //! get view matrix
  CGE::Matrix getViewMat() {return camTrafo_ * camRot_;}
  //! set the filename
  inline void setFilename(std::string filename) {filename_ = filename;}
  //! get the filename
  inline std::string getFilename() {return filename_;}
protected:
  //! Constructor
  Graphic();
  //! the render method
  void render_();
  //! the instance
  static Graphic* gra_;
  //! the scene
  //std::vector<MeshGeo::Mesh*> meshes_;
  CGE::Scene scene_;
  //! the camera transformation
  CGE::Matrix camTrafo_;
  //! the camera rotation
  CGE::Matrix camRot_;
  //! the current model
  MeshGeo::Model* currModel_;
  //! the filename of the loaded scene
  std::string filename_;
};

#endif

