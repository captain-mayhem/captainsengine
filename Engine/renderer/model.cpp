
#include <iostream>
#include <fstream>

#include "../system/engine.h"
#include "renderer.h"

#include "model.h"

using std::ifstream;
using std::ios;

using Graphics::Model;

Model::Model(){
  vb_ = NULL;
}

Model::~Model(){
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
