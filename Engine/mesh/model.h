
#ifndef MODEL_H
#define MODEL_H

#include "../system/gameobject.h"

namespace CGE{
class BoundingObject;
class Texture;
}

namespace MeshGeo{

class Mesh;

#define MAX_TEXTURES 8
#define MAX_ATTRIBS 8

class Model : public CGE::GameObject{
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
  inline void setBoundingObject(CGE::BoundingObject* obj) {boundingObj_ = obj;}
  //! Get the bounding object
  inline CGE::BoundingObject* getBoundingObject() const {return boundingObj_;}
  //! Assign a texture
  void assignTexture(CGE::Texture* tex, int stage);
  //! Get a texture
  CGE::Texture* getTexture(int stage) const {return textures_[stage];}
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
  //! The model attributes
  int attribs_[MAX_ATTRIBS];
};

};

#endif

