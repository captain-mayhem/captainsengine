
#ifndef MODEL_H
#define MODEL_H

#include <cge_export.h>

#include "../renderer/scenenode.h"
#include "../mesh/mesh.h"

namespace CGE{
class BoundingObject;
class Texture;
namespace Gfx{
class DrawContext;
class Texture;
}
}

namespace CGE{

#define MAX_TEXTURES 8
#define MAX_ATTRIBS 8

class CGE_API Model : public CGE::SceneNode{
public:
  //! Default constructor
  Model();
  //! Copy constructor
  Model(const Model& m);
  //! Initializes model with a mesh
  Model(Mesh* mesh);
  //! Destructor
  virtual ~Model();
  //! Render the model with the old renderer
  void render(Mesh::DrawPart part);
  //! renders the model through a render device in the context's model matrix
  /*! The mesh's materials decide the textures. Submeshes without a
      material get the model's device texture (stage 0, assignDeviceTexture)
      or, without one, the context's texture. */
  void render(Gfx::DrawContext& ctx, Mesh::DrawPart part);
  //! Setup material
  void setupMaterial();
  //! Reset material
  void resetMaterial();
  //! Get the model
  Mesh* getMesh() const {return mesh_;}
  //! Set a bounding object
  inline void setBoundingObject(CGE::BoundingObject* obj) {boundingObj_ = obj;}
  //! Get the bounding object
  inline CGE::BoundingObject* getBoundingObject() const {return boundingObj_;}
  //! Assign a texture
  void assignTexture(CGE::Texture* tex, int stage);
  //! Get a texture
  CGE::Texture* getTexture(int stage) const {return textures_[stage];}
  //! Assign a texture on the render device; it isn't owned by the model
  void assignDeviceTexture(Gfx::Texture* tex, int stage) {deviceTextures_[stage] = tex;}
  //! Get a texture on the render device
  Gfx::Texture* getDeviceTexture(int stage) const {return deviceTextures_[stage];}
  //! Set an attribute
  void setAttrib(int position, int value) {attribs_[position] = value;}
  //! Get an attribute
  int getAttrib(int position) {return attribs_[position];}
protected:
  //! The mesh
  Mesh* mesh_;
  //! The corresponding bounding object
  CGE::BoundingObject* boundingObj_;
  //! Assigned textures
  CGE::Texture* textures_[MAX_TEXTURES];
  //! Assigned textures on the render device
  Gfx::Texture* deviceTextures_[MAX_TEXTURES];
  //! The model attributes
  int attribs_[MAX_ATTRIBS];
};

};

#endif

