#include "scenenode.h"

using namespace CGE;

unsigned int SceneNode::idCount_ = 0;

SceneNode::SceneNode(Type t) : mType(t){
  trafo_ = CGE::Matrix(CGE::Matrix::Identity);
  id_ = ++idCount_;
}
/*
SceneNode::GameObject(SceneNode const& g){
  trafo_ = g.trafo_;
  //! the copy of an object is another object
  id_ = ++idCount_;
}
*/
SceneNode::~SceneNode(){
}