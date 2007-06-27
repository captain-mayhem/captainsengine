
#include "scene.h"
#include "texture.h"
#include "../system/engine.h"
#include "../mesh/model.h"
#include "../mesh/mesh.h"
#include "../math/ray.h"

#include <iostream>
#include <fstream>

using namespace Graphics;
using std::list;
using std::ofstream;
using std::ifstream;
using MeshGeo::Model;
using MeshGeo::Mesh;
using Math::BoundingObject;
using Math::Matrix;
using Math::Ray;

Scene::Scene(){
  version_ = 1;
}

Scene::~Scene(){
  list<MeshGeo::Model*>::iterator iter;
  for (iter = models_.begin(); iter != models_.end(); iter++){
    SAFE_DELETE(*iter);
  };
  models_.clear();
  for (unsigned i = 0; i < meshes_.size(); i++){
    SAFE_DELETE(meshes_[i]);
  }
  meshes_.clear();
  for (unsigned i = 0; i < textures_.size(); i++){
    SAFE_DELETE(textures_[i]);
  }
  textures_.clear();
}

void Scene::render(){
  list<MeshGeo::Model*>::iterator iter;
  for (iter = models_.begin(); iter != models_.end(); iter++){
    (*iter)->setupMaterial();
    (*iter)->render();
    (*iter)->resetMaterial();
  }
}

//! Get a model
Model* Scene::getModel(const int id){
  list<MeshGeo::Model*>::iterator iter;
  int count = 0;
  for (iter = models_.begin(); iter != models_.end(); iter++){
    if (count == id)
      return *iter;
    count++;
  };
  return NULL;
}

//! save the scene
void Scene::save(const std::string& filename) const{
  ofstream out(filename.c_str());
  if (!out)
    return;
  //version
  out.write((char*)&version_, sizeof(version_));
  //meshes
  unsigned size = meshes_.size();
  out.write((char*)&size, sizeof(size));
  for (unsigned i = 0; i < size; i++){
    string name = meshes_[i]->getFilename();
    unsigned length = name.size();
    out.write((char*)&length, sizeof(length));
    out.write(name.c_str(), sizeof(char)*length);
  }
  //textures
  size = textures_.size();
  out.write((char*)&size, sizeof(size));
  for (unsigned i = 0; i < size; i++){
    string name = textures_[i]->getFilename();
    unsigned length = name.size();
    out.write((char*)&length, sizeof(length));
    out.write(name.c_str(), sizeof(char)*length);
  }
  //models
  size = models_.size();
  out.write((char*)&size, sizeof(size));
  list<MeshGeo::Model*>::const_iterator iter;
  for (iter = models_.begin(); iter != models_.end(); iter++){
    //mesh link
    Mesh* mesh = (*iter)->getMesh();
    unsigned idx;
    for (idx = 0; idx < meshes_.size(); idx++){
      if (mesh == meshes_[idx])
        break;
    }
    out.write((char*)&idx, sizeof(idx));
    //texture links
    for (int i = 0; i < MAX_TEXTURES; i++){
      Texture* tex = (*iter)->getTexture(i);
      unsigned idx;
      if (tex == NULL){
        idx = UINT_MAX;
      }
      else{
        for (idx = 0; idx < textures_.size(); idx++){
          if (tex == textures_[idx])
            break;
        }
      }
      out.write((char*)&idx, sizeof(idx));
    }
    //trafo
    const float* mat = (*iter)->getTrafo().getData();
    out.write((char*)mat, 16*sizeof(float));
  }
  out.close();
}

//! load the scene
void Scene::load(const std::string& filename){
  ifstream in(filename.c_str());
  if (!in)
    return;
  char buffer[256];
  //version
  in.read((char*)&version_, sizeof(version_));
  //read and rebuild meshes
  unsigned size;
  in.read((char*)&size, sizeof(size));
  for (unsigned i = 0; i < size; i++){
    unsigned length;
    in.read((char*)&length, sizeof(length));
    in.read(buffer, length*sizeof(char));
    buffer[length] = '\0';
    string name = string(buffer);
    Mesh* msh = new Mesh();
    if (!msh->loadFromFile(name)){
      System::Log << "cannot load file";
      return;
    }
    msh->buildVBO();
    meshes_.push_back(msh);
  }
  //read and rebuild textures
  in.read((char*)&size, sizeof(size));
  for (unsigned i = 0; i < size; i++){
    unsigned length;
    in.read((char*)&length, sizeof(length));
    in.read(buffer, length*sizeof(char));
    buffer[length] = '\0';
    string name = string(buffer);
    Texture* tex = Texture::create(name);
    textures_.push_back(tex);
  }
  //read models
  in.read((char*)&size, sizeof(size));
  for (unsigned i = 0; i < size; i++){
    //mesh link
    unsigned idx;
    in.read((char*)&idx, sizeof(idx));
    Mesh* msh = meshes_[idx];
    Model* mdl = new Model(msh);
    models_.push_back(mdl);
    //texture links
    for (int i = 0; i < MAX_TEXTURES; i++){
      in.read((char*)&idx, sizeof(idx));
      if (idx != UINT_MAX){
        Texture* tex = textures_[idx];
        mdl->assignTexture(tex,i);
      }
    }
    //trafo
    float mat[16];
    in.read((char*)mat, 16*sizeof(float));
    mdl->setTrafo(Matrix(mat));
  }
  in.close();
}

//! Get the model that is the nearest one on the ray
MeshGeo::Model* Scene::pickModel(const Ray& ray) const {
  list<MeshGeo::Model*>::const_iterator iter;
  //std::cerr << ray.getOrigin() << " " << ray.getDirection() << "\n";
  for (iter = models_.begin(); iter != models_.end(); iter++){
    Model* mdl = *iter;
    BoundingObject* bound = mdl->getBoundingObject();
    BoundingObject* tmp = bound->copy();
    tmp->transform(mdl->getTrafo());
    if (tmp->hit(ray)){
      delete tmp;
      return mdl;
    }
    delete tmp;
  }
  return NULL;
}

