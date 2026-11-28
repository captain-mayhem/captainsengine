#ifndef GRAPHIC_H
#define GRAPHIC_H

#include <map>
#include <memory>
#include <vector>
#include "math/matrix.h"
#include "system/engine.h"
#include "renderer/scene.h"
#include "camera.h"

namespace CGE{
  class ModelFile;
  class RenderTechnique;
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
  //! true for the files of addModelFile (.gltf, .glb)
  static bool isModelFile(const std::string& filename);
  //! loads a glTF model (.gltf, .glb) and adds it to the scene, selected and in view
  /*! False if it can't be read; getMessage says why. */
  bool addModelFile(const std::string& filename);
  //! adds another copy of a loaded glTF model (getModelFiles) to the scene, selected
  void addModelFileInstance(unsigned idx);
  //! the glTF models loaded, in their order
  const std::vector<std::unique_ptr<CGE::ModelFile>>& getModelFiles() const {return mModelFiles;}
  //! the glTF model a node of the scene is a copy of; nullptr for other nodes
  CGE::ModelFile* getModelFileOf(CGE::SceneNode* node) const;
  //! takes a copy of a node of the scene (key C) as another node of the scene, selected
  void copyNode(CGE::SceneNode* node);
  //! removes a node from the scene and deletes it
  void deleteNode(CGE::SceneNode* node);
  //! moves the camera so that the node is in view, and orbits around it then
  void frame(CGE::SceneNode* node);
  //! lights the scene from the camera while it has no lights of its own
  void setDefaultLights(bool on) {mDefaultLights = on;}
  bool hasDefaultLights() const {return mDefaultLights;}
  //! the last message for the user (e.g. why a file couldn't be loaded), empty if none
  const std::string& getMessage() const {return mMessage;}
  void setMessage(const std::string& message) {mMessage = message;}
  //! add a texture to the editor
  void addTexture(const std::string& filename);
  //! add the meshes, textures, lights and models of a scene file
  void loadScene(const std::string& filename);
  //! set transformation of a mesh
  void setTrafo(const int id, const CGE::Matrix& mat);
  //! get the scene
  CGE::Scene& getScene() {return scene_;}
  //! set the selected node (a model, or a glTF model's copy)
  void setCurrNode(CGE::SceneNode* node) {currNode_ = node;}
  //! the selected node; nullptr for none
  CGE::SceneNode* getCurrNode() const {return currNode_;}
  //! the selected node if it is a model (with attributes and textures), else nullptr
  CGE::Model* getCurrModel() const;
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
  //! the glTF models loaded; declared before the scene, so its copies of their nodes go first
  std::vector<std::unique_ptr<CGE::ModelFile>> mModelFiles;
  //! the scene's nodes that are copies of glTF models, and those models
  std::map<CGE::SceneNode*, CGE::ModelFile*> mModelFileNodes;
  //! the scene
  CGE::Scene scene_;
  //! the selected node
  CGE::SceneNode* currNode_;
  //! light from the camera while the scene has no lights
  bool mDefaultLights;
  //! for the user
  std::string mMessage;
  //! the filename of the loaded scene
  std::string filename_;
  //! the rendering technique
  CGE::RenderTechnique* mTechnique;
};

#endif

