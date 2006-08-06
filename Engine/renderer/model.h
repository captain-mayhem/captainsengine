#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>

#include "../math/matrix.h"
#include "vertexbuffer.h"

namespace Graphics{

class VertexBuffer;
  
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
  //! load from a Hero-Engine-Model (binary format)
  void loadFromHMB(const std::string& filename);
  //! activate the model for rendering
  inline void activate() {vb_->activate();}
  //! draw a model
  inline void draw() {vb_->draw(VB_Triangles);}
  //! clone a model
  /*! cloning a model means that the vertexbuffer
   * is not copied but shared 
   */
  Model clone();
  //! set a transformation
  inline void setTransform(const Math::Matrix mat) {transform_ = mat;}
  //! get a transformation
  inline Math::Matrix getTransform() {return transform_;}
private:
  //! the vertices
  std::vector<VerTexNorm> vertices_;
  //! the indices
  std::vector<unsigned short> indices_;
  //! the vertexbuffer
  VertexBuffer* vb_;
  //! the transformation
  Math::Matrix transform_;
  //! if the model was cloned
  bool cloned_;
};

}

#endif

