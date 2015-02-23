#ifndef CGE_SCENENODE_H
#define CGE_SCENENODE_H

#include <math/matrix.h>
#include <vector>

namespace CGE{

class SceneNode{
public:
  enum Type{
    MESH,
  };
  //! Constructor
  SceneNode(Type type);
  //! Copy constructor
  //SceneNode(GameObject const& g);
  //! Destructor
  virtual ~SceneNode();
  //! set transformation
  inline void setTrafo(const CGE::Matrix& mat) { trafo_ = mat; }
  //! get transformation
  inline CGE::Matrix const& getTrafo() const { return trafo_; }
  //! get id
  inline unsigned int getID() const { return id_; }
  //! set id
  void setID(unsigned id) { id_ = id; }
  //! set id count
  static void setIDCount(unsigned start) { idCount_ = start; }
  //! get id count
  static unsigned getIDCount(){ return idCount_; }
  //! get the type
  Type getType() { return mType; }
protected:
  //! the node type
  Type mType;
  //! The id counter
  static unsigned int idCount_;
  //! The object id
  unsigned int id_;
  //! The transformation
  CGE::Matrix trafo_;
  //! the children
  std::vector<SceneNode*> mChildren;
};

}

#endif
