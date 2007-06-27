
#ifndef MODEL_H
#define MODEL_H

#include "../system/gameobject.h"

namespace Math{
class BoundingObject;
}

namespace Graphics{
class Texture;
}

namespace MeshGeo{

class Mesh;

#define MAX_TEXTURES 8
  
class Model : public System::GameObject{
public:
  //! Default constructor
  Model();
  //! Initializes model with a mesh
  Model(Mesh* mesh);
  //! Destructor
  virtual ~Model();
  //! Render the model
  void render();
  //! Setup material
  void setupMaterial();
  //! Reset material
  void resetMaterial();
  //! Get the model
  Mesh* getMesh() const {return mesh_;}
  //! Set a bounding object
  inline void setBoundingObject(Math::BoundingObject* obj) {boundingObj_ = obj;}
  //! Get the bounding object
  inline Math::BoundingObject* getBoundingObject() const {return boundingObj_;}
  //! Assign a texture
  void assignTexture(Graphics::Texture* tex, int stage);
  //! Get a texture
  Graphics::Texture* getTexture(int stage) const {return textures_[stage];}
protected:
  //! The mesh
  Mesh* mesh_;
  //! The corresponding bounding object
  Math::BoundingObject* boundingObj_;
  //! Assigned textures
  Graphics::Texture* textures_[MAX_TEXTURES];
};

};

#endif

