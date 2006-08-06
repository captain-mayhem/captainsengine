
#include <iostream>
#include <fstream>

#include "../system/engine.h"
#include "renderer.h"

#include "model.h"

using std::ifstream;
using std::ios;
using std::vector;

using Graphics::Model;

Model::Model(){
  vb_ = NULL;
  cloned_ = false;
}


// copy constructor
Model::Model(const Model& m){
  indices_ = vector<unsigned short>(m.indices_);
  vertices_ = vector<VerTexNorm>(m.vertices_);
  transform_ = m.transform_;
  cloned_ = true;
  vb_ = m.vb_;
}

Model::~Model(){
  //delete only original models
  if (cloned_)
    vb_ = NULL;
  else
    SAFE_DELETE(vb_);
    
}

void Model::loadFromHMB(const std::string& filename){
  ifstream in(filename.c_str(), ios::binary);
  unsigned size;
  in.read((char*)&size, sizeof(size));
  for (unsigned i = 0; i < size; i++){
    VerTexNorm vert;
    in.read((char*)&vert, sizeof(vert));
    vertices_.push_back(vert);
  }
  in.read((char*)&size, sizeof(size));
  for (unsigned i = 0; i < size; i++){
    unsigned short idx;
    in.read((char*)&idx, sizeof(idx));
    indices_.push_back(idx);
  }
  in.close();

  //build vertexbuffer
  vb_ = System::Engine::instance()->getRenderer()->createVertexBuffer();
  vb_->create(VB_POSITION | VB_TEXCOORD, vertices_.size(), indices_.size());
  vb_->lockVertexPointer();
  for (unsigned i = 0; i < vertices_.size(); i++){
    vb_->setPosition(i, Vertex(vertices_[i].v));
    vb_->setTexCoord(i, Vec2f(vertices_[i].t[0], vertices_[i].t[1]));
  }
  vb_->unlockVertexPointer();
  short* idx = vb_->lockIndexPointer();
  for (unsigned i = 0; i < indices_.size(); i++){
    idx[i] = indices_[i];
  }
  vb_->unlockVertexPointer();
}

Model Model::clone(){
  Model mod;
  mod.indices_ = vector<unsigned short>(indices_);
  mod.vertices_ = vector<VerTexNorm>(vertices_);
  mod.vb_ = vb_;
  mod.transform_ = transform_;
  mod.cloned_ = true;
  return mod;
}

