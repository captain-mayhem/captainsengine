
#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <list>
#include <string>

namespace CGE{
class Ray;
class Model;
class Mesh;
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
  void addModel(Model* model) {models_.push_back(model);}
  //! Delete a model
  void deleteModel(const unsigned id);
  //! Get a model
  Model* getModel(const unsigned id);
  //! Add a texture
  unsigned addTexture(Texture* tex) {textures_.push_back(tex); return (unsigned)textures_.size();}
  //! Get all textures
  std::vector<CGE::Texture*> getTextures() const {return textures_;}
  //! render the scene
  void render();
  //! save the scene
  void save(const std::string& filename) const;
  //! load the scene
  void load(const std::string& filename);
  //! Get the model that is the nearest one on the ray
  Model* pickModel(const CGE::Ray& ray) const;
  //! get the models
  const std::list<Model*>& getModels() const {return models_;}
protected:
  //! The scene version number
  int version_;
  //! The models (components the scene consists of)
  std::list<Model*> models_;
  //! The meshes
  std::vector<Mesh*> meshes_;
  //! The textures
  std::vector<Texture*> textures_;
};

}

#endif

