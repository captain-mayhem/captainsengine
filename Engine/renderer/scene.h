
#ifndef SCENE_H
#define SCENE_H

#include <cge_export.h>

#include <vector>
#include <list>
#include <memory>
#include <string>
#include <mesh/model.h>

namespace CGE{
class Ray;
class Mesh;
class Light;
class Camera;
namespace Gfx{
class DrawContext;
class RenderDevice;
class Texture;
}
}

namespace CGE{

class Texture;
  
class CGE_API Scene{
public:
  Scene();
  //! owns its models, meshes and textures
  Scene(const Scene&) = delete;
  Scene& operator=(const Scene&) = delete;
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
  //! a texture the scene loaded onto a render device, by its file name in the scene file
  /*! E.g. "textures/world/door.jpg"; nullptr if the scene has none of that name. */
  Gfx::Texture* getDeviceTexture(const std::string& name) const;
  //! Add a light
  unsigned addLight(Light* light) { mLights.push_back(light); return (unsigned)mLights.size(); }
  //! Get the lights
  std::vector<Light*>& getLights() { return mLights; }
  //! render the scene with the old renderer
  void render(Mesh::DrawPart part=Mesh::DRAW_ALL);
  //! renders the models through a render device (in a pass)
  /*! The context's projection, view and lights stay as they are; see
      ForwardRenderer for a pass with the scene's camera and lights. */
  void render(Gfx::DrawContext& ctx, Mesh::DrawPart part=Mesh::DRAW_ALL);
  //! save the scene
  void save(const std::string& filename) const;
  //! deletes the models, meshes, textures and lights
  /*! Also what is on a render device, so call it before the device goes
      when the scene lives longer. */
  void clear();
  //! load the scene
  /*! With a device, the meshes get their buffers on it and the textures
      are loaded onto it and assigned to the models as device textures;
      nothing is made for the old renderer. Without one, the old renderer's
      buffers and textures are made. */
  void load(const std::string& filename, Gfx::RenderDevice* device = nullptr);
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
  //! The textures on a render device, and their file names in the scene file
  std::vector<std::unique_ptr<Gfx::Texture>> mDeviceTextures;
  std::vector<std::string> mDeviceTextureNames;
  //! The lights
  std::vector<Light*> mLights;
  //! active camera
  Camera* mActiveCam;
};

}

#endif

