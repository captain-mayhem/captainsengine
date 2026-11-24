
#include "scenenode.h"

#include <algorithm>

#include "../math/matrixstack.h"
#include "drawcontext.h"

using namespace CGE;

unsigned int SceneNode::idCount_ = 0;

SceneNode::SceneNode(Type t) : mType(t), mParent(nullptr){
  trafo_ = CGE::Matrix(CGE::Matrix::Identity);
  id_ = ++idCount_;
}

SceneNode::SceneNode(const SceneNode& node) : mType(node.mType), trafo_(node.trafo_), mName(node.mName), mParent(nullptr){
  //the copy of an object is another object
  id_ = ++idCount_;
  for (SceneNode* child : node.mChildren)
    addChild(child->clone());
}

SceneNode::~SceneNode(){
  for (SceneNode* child : mChildren)
    delete child;
}

SceneNode* SceneNode::clone() const{
  return new SceneNode(*this);
}

Matrix SceneNode::getWorldTrafo() const{
  Matrix result = trafo_;
  for (const SceneNode* node = mParent; node; node = node->mParent)
    result = node->trafo_*result;
  return result;
}

void SceneNode::addChild(SceneNode* child){
  if (child->mParent)
    child->mParent->removeChild(child);
  child->mParent = this;
  mChildren.push_back(child);
}

bool SceneNode::removeChild(SceneNode* child){
  auto iter = std::find(mChildren.begin(), mChildren.end(), child);
  if (iter == mChildren.end())
    return false;
  mChildren.erase(iter);
  child->mParent = nullptr;
  return true;
}

SceneNode* SceneNode::findNode(const std::string& name){
  if (mName == name)
    return this;
  for (SceneNode* child : mChildren){
    SceneNode* found = child->findNode(name);
    if (found)
      return found;
  }
  return nullptr;
}

void SceneNode::render(Gfx::DrawContext& ctx, Mesh::DrawPart part){
  ctx.model().push();
  ctx.model().multiply(trafo_);
  renderSelf(ctx, part);
  for (SceneNode* child : mChildren)
    child->render(ctx, part);
  ctx.model().pop();
}
