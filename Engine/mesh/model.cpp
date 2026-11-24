
#include "model.h"
#include "../math/matrix.h"
#include "../math/bsphere.h"
#include "../system/engine.h"
#include "../mesh/mesh.h"
#include "../math/matrixstack.h"
#include "../renderer/drawcontext.h"

#include <cassert>

using namespace CGE;

Model::Model() : SceneNode(MESH){
  trafo_ = Matrix(Matrix::Identity);
  mesh_ = NULL;
  boundingObj_ = NULL;
  for (int i = 0; i < MAX_TEXTURES; i++){
    deviceTextures_[i] = nullptr;
  }
  for (int i = 0; i < MAX_ATTRIBS; i++){
    attribs_[i] = 0;
  }
}

Model::Model(Mesh* mesh) : SceneNode(MESH){
  trafo_ = Matrix(Matrix::Identity);
  mesh_ = mesh;
  BBox box = mesh_->getExtents();
  Vector3D center = (box.getMin()+box.getMax())/2.0f;
  float radius = (box.getMax()-box.getMin()).length()/2.0f;
  boundingObj_ = new BSphere(center, radius);
  for (int i = 0; i < MAX_TEXTURES; i++){
    deviceTextures_[i] = nullptr;
  }
  for (int i = 0; i < MAX_ATTRIBS; i++){
    attribs_[i] = 0;
  }
}

//! Copy constructor
Model::Model(const Model& m) : SceneNode(m){
  //trafo_ = m.trafo_;
  mesh_ = m.mesh_;
  boundingObj_ = m.boundingObj_ ? m.boundingObj_->copy() : nullptr;
  for (int i = 0; i < MAX_TEXTURES; i++){
    deviceTextures_[i] = m.deviceTextures_[i];
  }
  for (int i = 0; i < MAX_ATTRIBS; i++){
    attribs_[i] = m.attribs_[i];
  }
}

Model::~Model(){
  SAFE_DELETE(boundingObj_);
}

Model* Model::clone() const{
  return new Model(*this);
}

void Model::renderSelf(Gfx::DrawContext& ctx, Mesh::DrawPart part){
  assert(mesh_ && "A mesh needs to be assigned to the model");
  if (deviceTextures_[0]){
    Gfx::Texture* texture = ctx.getTexture();
    Gfx::Sampler* sampler = ctx.getSampler();
    ctx.setTexture(deviceTextures_[0]);
    mesh_->draw(ctx, part);
    ctx.setTexture(texture, sampler);
  }
  else
    mesh_->draw(ctx, part);
}
