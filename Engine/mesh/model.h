
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
#define MAX_ATTRIBS 8

class Model : public System::GameObject{
public:
  //! Default constructor
  Model();
  //! Copy constructor
  Model(const Model& m);
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
  //! Set an attribute
  void setAttrib(int position, int value) {attribs_[position] = value;}
  //! Get an attribute
  int getAttrib(int position) {return attribs_[position];}
protected:
  //! The mesh
  Mesh* mesh_;
  //! The corresponding bounding object
  Math::BoundingObject* boundingObj_;
  //! Assigned textures
  Graphics::Texture* textures_[MAX_TEXTURES];
  //! The model attributes
  int attribs_[MAX_ATTRIBS];
};

};

#endif

