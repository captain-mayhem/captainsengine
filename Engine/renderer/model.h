#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>

#include "../math/matrix.h"
#include "vertexbuffer.h"

namespace CGE_OLD{

class VertexBuffer;
class ModelInstance;
  
struct VerTexNorm{
  float v[3];
  float t[2];
  float n[3];
};


//! A model
class Model{
public:
  //! constructor
  Model();
  //! copy constructor
  Model(const Model& m);
  //! destructor
  ~Model();
  //! load from a Captains-Engine-Model (binary format)
  void loadFromHMB(const std::string& filename);
  //! activate the model for rendering
  inline void activate() {vb_->activate();}
  //! draw a model
  inline void draw() {vb_->draw(CGE::VB_Triangles, inds_);}
  //! clone a model
  /*! cloning a model means that the vertexbuffer
   * is not copied but shared 
   */
  ModelInstance clone();
  //! set a transformation
  inline void setTransform(const CGE::Matrix mat) {transform_ = mat;}
  //! get a transformation
  inline CGE::Matrix getTransform() {return transform_;}
private:
  //! the vertices
  std::vector<VerTexNorm> vertices_;
  //! the indices
  std::vector<unsigned short> indices_;
  //! the vertexbuffer
  CGE::VertexBuffer* vb_;
  //! the indexbuffer
  CGE::IndexBuffer* inds_;
  //! the transformation
  CGE::Matrix transform_;
  //! if the model was cloned
  bool cloned_;
};


//! A model instance
class ModelInstance{
public:
  //! default constructor
  ModelInstance();
  //! constructor
  ModelInstance(CGE_OLD::Model* model);
  //! copy constructor
  ModelInstance(const ModelInstance& m);
  //! destructor
  ~ModelInstance();
  //! activate the model for rendering
  inline void activate() {model_->activate();}
  //! draw a model
  inline void draw() {model_->draw();}
  //! set a transformation
  inline void setTransform(const CGE::Matrix mat) {transform_ = mat;}
  //! get a transformation
  inline CGE::Matrix getTransform() {return transform_;}
private:
  //! the transformation
  CGE::Matrix transform_;
  //! a pointer to the model
  CGE_OLD::Model* model_;
};


}

#endif

