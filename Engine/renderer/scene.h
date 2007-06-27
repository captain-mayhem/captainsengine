
#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <list>

namespace Math{
class Ray;
}

namespace MeshGeo{
class Model;
class Mesh;
}

namespace Graphics{

class Texture;
  
class Scene{
public:
  Scene();
  virtual ~Scene();
  //! Add a mesh
  unsigned addMesh(MeshGeo::Mesh* mesh) {meshes_.push_back(mesh); return (unsigned)meshes_.size();}
  //! Get all meshes
  std::vector<MeshGeo::Mesh*> getMeshes() const {return meshes_;}
  //! Add a model
  void addModel(MeshGeo::Model* model) {models_.push_back(model);}
  //! Get a model
  MeshGeo::Model* getModel(const int id);
  //! Add a texture
  unsigned addTexture(Texture* tex) {textures_.push_back(tex); return (unsigned)textures_.size();}
  //! Get all textures
  std::vector<Graphics::Texture*> getTextures() const {return textures_;}
  //! render the scene
  void render();
  //! save the scene
  void save(const std::string& filename) const;
  //! load the scene
  void load(const std::string& filename);
  //! Get the model that is the nearest one on the ray
  MeshGeo::Model* pickModel(const Math::Ray& ray) const;
protected:
  //! The scene version number
  int version_;
  //! The models (components the scene consists of)
  std::list<MeshGeo::Model*> models_;
  //! The meshes
  std::vector<MeshGeo::Mesh*> meshes_;
  //! The textures
  std::vector<Texture*> textures_;
};

}

#endif

