#ifndef GRAPHIC_H
#define GRAPHIC_H

#include <vector>
#include "math/matrix.h"
#include "system/engine.h"
#include "renderer/scene.h"
#include "renderer/rendertarget.h"
#include "camera.h"

namespace MeshGeo{
class Mesh;
}

class Graphic{
public:
  ~Graphic();
  static void init();
  static void render();
  static void resize(int width, int height);
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
  //! get the scene
  CGE::Scene& getScene() {return scene_;}
  //! set the current model
  void setCurrModel(CGE::Model* mdl) {currModel_ = mdl;}
  //! get the current model
  CGE::Model* getCurrModel() const {return currModel_;}
  //! set the filename
  inline void setFilename(std::string filename) {filename_ = filename;}
  //! get the filename
  inline std::string getFilename() {return filename_;}
  //! get the camera
  EditCam& getCam() { return mCam; }
protected:
  //! Constructor
  Graphic();
  //! the render method
  void render_();
  //! the resize method
  void resize_(int width, int height);
  //! the instance
  static Graphic* gra_;
  //! the camera
  EditCam mCam;
  //! the scene
  CGE::Scene scene_;
  //! the current model
  CGE::Model* currModel_;
  //! the filename of the loaded scene
  std::string filename_;
  //! the fbo
  CGE::RenderTarget* mRT;
};

#endif

