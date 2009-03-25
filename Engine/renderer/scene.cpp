
#include "scene.h"
#include "texture.h"
#include "../system/engine.h"
#include "../system/utilities.h"
#include "../system/file.h"
#include "../mesh/model.h"
#include "../mesh/mesh.h"
#include "../math/ray.h"

#include <iostream>
#include <fstream>
#include <float.h>

using namespace Graphics;
using std::list;
using std::ofstream;
using std::ifstream;
using std::ios;
using std::max;
using MeshGeo::Model;
using MeshGeo::Mesh;
using CGE::BoundingObject;
using CGE::Matrix;
using CGE::Ray;
using System::Utilities;

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
Model* Scene::getModel(const unsigned id){
  list<MeshGeo::Model*>::iterator iter;
  //int count = 0;
  for (iter = models_.begin(); iter != models_.end(); iter++){
    if ((*iter)->getID() == id)
      return *iter;
    //count++;
  };
  return NULL;
}

//! save the scene
void Scene::save(const std::string& filename) const{
  ofstream out(filename.c_str(), ios::binary);
  if (!out)
    return;
  //version
  out.write((char*)&version_, sizeof(version_));
  //meshes
  size_t size = meshes_.size();
  out.write((char*)&size, sizeof(size));
  for (unsigned i = 0; i < size; i++){
    string name = meshes_[i]->getFilename();
#ifdef UNIX
    Utilities::replaceWith(name, '/', '\\');
#endif
    size_t length = name.size();
    out.write((char*)&length, sizeof(length));
    out.write(name.c_str(), (std::streamsize) sizeof(char)*length);
  }
  //textures
  size = textures_.size();
  out.write((char*)&size, sizeof(size));
  for (unsigned i = 0; i < size; i++){
    string name = textures_[i]->getFilename();
#ifdef UNIX
    Utilities::replaceWith(name, '/', '\\');
#endif
    size_t length = name.size();
    out.write((char*)&length, sizeof(length));
    out.write(name.c_str(), sizeof(char)*length);
  }
  //models
  size = models_.size();
  out.write((char*)&size, sizeof(size));
  list<MeshGeo::Model*>::const_iterator iter;
  for (iter = models_.begin(); iter != models_.end(); iter++){
    //model id
    unsigned id = (*iter)->getID();
    out.write((char*)&id, sizeof(id));
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
    //attributes
    for (int i = 0; i < MAX_ATTRIBS; i++){
      int att = (*iter)->getAttrib(i);
      out.write((char*)&att, sizeof(att));
    }
  }
  out.close();
}

//! load the scene
void Scene::load(const std::string& filename){
  ifstream in(filename.c_str(), ios::binary);
  if (!in)
    return;
  std::string cwd = System::Filesystem::getCwd();
  cwd += "\\";
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
#ifdef UNIX
    Utilities::replaceWith(name, '\\', '/');
#endif
    Mesh* msh = new Mesh();
    if (!msh->loadFromFile(cwd+name)){
      System::Log << "cannot load file";
      return;
    }
    buffer[length] = -52;
    msh->buildVBO();
    meshes_.push_back(msh);
  }
  //read and rebuild textures
  in.read((char*)&size, sizeof(size));
  for (unsigned i = 0; i < size; i++){
    unsigned length;
    in.read((char*)&length, sizeof(length));
    //System::Log << in.bad() << " " << in.eof();
    in.read(buffer, length*sizeof(char));
    buffer[length] = '\0';
    string name = string(buffer);
#ifdef UNIX
    Utilities::replaceWith(name, '\\', '/');
#endif
    Texture* tex = Texture::create(cwd+name);
    textures_.push_back(tex);
  }
  //read models
  in.read((char*)&size, sizeof(size));
  for (unsigned i = 0; i < size; i++){
    //model id
    unsigned id;
    in.read((char*)&id, sizeof(id));
    System::GameObject::setIDCount(max(id+1, System::GameObject::getIDCount()));
    //mesh link
    unsigned idx;
    in.read((char*)&idx, sizeof(idx));
    Mesh* msh = meshes_[idx];
    Model* mdl = new Model(msh);
    mdl->setID(id);
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
    //attributes
    for (int i = 0; i < MAX_ATTRIBS; i++){
      int attrib;
      in.read((char*)&attrib, sizeof(attrib));
      mdl->setAttrib(i, attrib);
    }
  }
  in.close();
}

//! Get the model that is the nearest one on the ray
MeshGeo::Model* Scene::pickModel(const Ray& ray) const {
  list<MeshGeo::Model*>::const_iterator iter;
  //std::cerr << ray.getOrigin() << " " << ray.getDirection() << "\n";
  Model* nearest = NULL;
  float nearDistance = FLT_MAX;
  for (iter = models_.begin(); iter != models_.end(); iter++){
    Model* mdl = *iter;
    BoundingObject* bound = mdl->getBoundingObject();
    BoundingObject* tmp = bound->copy();
    tmp->transform(mdl->getTrafo());
    float currDistance = tmp->hit(ray);
    //Bounding object was hit
    if (currDistance < FLT_MAX){
      //determine real intersection point
      Ray r(ray);
      r.transform(mdl->getTrafo().inverse());
      currDistance = mdl->getMesh()->intersect(r);
    }
    if (currDistance < nearDistance){
      nearDistance = currDistance;
      nearest = mdl;
      //delete tmp;
      //return mdl;
    }
    delete tmp;
  }
  return nearest;
}

//! Delete a model
void Scene::deleteModel(const unsigned id){
  list<MeshGeo::Model*>::iterator iter;
  for (iter = models_.begin(); iter != models_.end(); iter++){
    if (id == (*iter)->getID()){
      models_.erase(iter);
      return;
    }
  }
}

