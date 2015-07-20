
#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <list>
#include <string>
#include <mesh/model.h>

namespace CGE{
class Ray;
class Mesh;
class Light;
class Camera;
}

namespace CGE{

class Texture;
  
class Scene{
public:
  Scene();
  virtual ~Scene();
  //! Add a mesh
  unsigned addMesh(Mesh* mesh) {meshes_.push_back(mesh); return (unsigned)meshes_.size();}
  //! Get all meshes
  std::vector<Mesh*> const& getMeshes() const {return meshes_;}
  //! Add a model
  void addModel(Model* model) {mNodes.push_back(model);}
  //! Delete a model
  void deleteModel(const unsigned id);
  //! Get a model
  Model* getModel(const unsigned id);
  //! Add a texture
  unsigned addTexture(Texture* tex) {textures_.push_back(tex); return (unsigned)textures_.size();}
  //! Get all textures
  std::vector<CGE::Texture*> getTextures() const {return textures_;}
  //! Add a light
  unsigned addLight(Light* light) { mLights.push_back(light); return (unsigned)mLights.size(); }
  //! Get the lights
  std::vector<Light*>& getLights() { return mLights; }
  //! render the scene
  void render(Mesh::DrawPart part=Mesh::DRAW_ALL);
  //! save the scene
  void save(const std::string& filename) const;
  //! load the scene
  void load(const std::string& filename);
  //! Get the model that is the nearest one on the ray
  Model* pickModel(const CGE::Ray& ray) const;
  //! get the models
  const std::list<SceneNode*>& getNodes() const {return mNodes;}
  //! set active camera
  void setActiveCam(Camera* cam) { mActiveCam = cam; }
  //! get active camera
  Camera* getActiveCam() { return mActiveCam; }
protected:
  //! The scene version number
  int version_;
  //! The nodes (components the scene consists of)
  std::list<SceneNode*> mNodes;
  //! The meshes
  std::vector<Mesh*> meshes_;
  //! The textures
  std::vector<Texture*> textures_;
  //! The lights
  std::vector<Light*> mLights;
  //! active camera
  Camera* mActiveCam;
};

}

#endif

