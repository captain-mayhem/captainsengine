
#include "scene.h"
#include "texture.h"
#include "../system/engine.h"
#include "../system/utilities.h"
#include "../system/file.h"
#include "../mesh/model.h"
#include "../mesh/mesh.h"
#include "../math/ray.h"
#include "../renderer/light.h"
#include "../renderer/renderer.h"
#include "../renderer/camera.h"

#include <iostream>
#include <fstream>
#include <float.h>
#include <limits.h>
#include <algorithm>

using namespace CGE;
using std::list;
using std::ofstream;
using std::ifstream;
using std::ios;
using std::max;
using CGE::Model;
using CGE::Mesh;
using CGE::BoundingObject;
using CGE::Matrix;
using CGE::Ray;
using CGE::Utilities;

TR_CHANNEL(CGE_Scene);

Scene::Scene() : mActiveCam(NULL){
  version_ = 1;
}

Scene::~Scene(){
  list<SceneNode*>::iterator iter;
  for (iter = mNodes.begin(); iter != mNodes.end(); iter++){
    SAFE_DELETE(*iter);
  };
  mNodes.clear();
  for (unsigned i = 0; i < meshes_.size(); i++){
    SAFE_DELETE(meshes_[i]);
  }
  meshes_.clear();
  for (unsigned i = 0; i < textures_.size(); i++){
    SAFE_DELETE(textures_[i]);
  }
  for (unsigned i = 0; i < mLights.size(); ++i){
    SAFE_DELETE(mLights[i]);
  }
  textures_.clear();
}

void Scene::render(Mesh::DrawPart part){
  Renderer* rend = Engine::instance()->getRenderer();
  list<SceneNode*>::iterator iter;
  for (iter = mNodes.begin(); iter != mNodes.end(); iter++){
    if ((*iter)->getType() == SceneNode::MESH){
      Model* mdl = (Model*)*iter;
      mdl->setupMaterial();
      mdl->render(part);
      mdl->resetMaterial();
    }
  }
}

//! Get a model
Model* Scene::getModel(const unsigned id){
  list<SceneNode*>::iterator iter;
  //int count = 0;
  for (iter = mNodes.begin(); iter != mNodes.end(); iter++){
    if ((*iter)->getID() == id){
      if ((*iter)->getType() == SceneNode::MESH)
        return (Model*)*iter;
      else
        return NULL;
    }
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
  //lights
  size = mLights.size();
  out.write((char*)&size, sizeof(size));
  for (unsigned i = 0; i < size; i++){
    Light* lt = mLights[i];
    char type = (char)lt->getType();
    out.write(&type, 1);
    out.write((const char*)lt->getPosition().data, 4 * sizeof(float));
    out.write((const char*)lt->getDirection().data, 3 * sizeof(float));
    float flt = lt->getCutoff();
    out.write((const char*)&flt, sizeof(float));
    out.write((const char*)lt->getColor().array, 4 * sizeof(float));
    flt = lt->getAttenuation();
    out.write((const char*)&flt, sizeof(float));
  }
  //models
  size = mNodes.size();
  out.write((char*)&size, sizeof(size));
  list<SceneNode*>::const_iterator iter;
  for (iter = mNodes.begin(); iter != mNodes.end(); iter++){
    //model id
    unsigned id = (*iter)->getID();
    out.write((char*)&id, sizeof(id));
    if ((*iter)->getType() != SceneNode::MESH)
      continue;
    Model* mdl = (Model*)*iter;
    //mesh link
    Mesh* mesh = mdl->getMesh();
    unsigned idx;
    for (idx = 0; idx < meshes_.size(); idx++){
      if (mesh == meshes_[idx])
        break;
    }
    out.write((char*)&idx, sizeof(idx));
    //texture links
    for (int i = 0; i < MAX_TEXTURES; i++){
      Texture* tex = mdl->getTexture(i);
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
      int att = mdl->getAttrib(i);
      out.write((char*)&att, sizeof(att));
    }
  }
  out.close();
}

//! load the scene
void Scene::load(const std::string& filename){
  TR_USE(CGE_Scene);
  ifstream in(filename.c_str(), ios::binary);
  if (!in)
    return;
  std::string cwd = CGE::Filesystem::getCwd();
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
      TR_ERROR("cannot load file %s", filename.c_str());
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
    //CGE::Log << in.bad() << " " << in.eof();
    in.read(buffer, length*sizeof(char));
    buffer[length] = '\0';
    string name = string(buffer);
#ifdef UNIX
    Utilities::replaceWith(name, '\\', '/');
#endif
    Texture* tex = Texture::create(cwd+name);
    textures_.push_back(tex);
  }
  //read lights
  in.read((char*)&size, sizeof(size));
  for (unsigned i = 0; i < size; ++i){
    char type;
    in.read(&type, 1);
    Vec4f pos;
    in.read((char*)pos.data, 4 * sizeof(float));
    Light* lt = new Light((Light::Type)type, pos);
    Vec3f dir;
    in.read((char*)dir.data, 3 * sizeof(float));
    lt->setDirection(dir);
    float flt;
    in.read((char*)&flt, sizeof(float));
    lt->setCutoff(flt);
    Color c;
    in.read((char*)c.array, 4 * sizeof(float));
    lt->setColor(c);
    in.read((char*)&flt, sizeof(float));
    lt->setAttenuation(flt);
    mLights.push_back(lt);
  }
  //read models
  in.read((char*)&size, sizeof(size));
  for (unsigned i = 0; i < size; i++){
    //model id
    unsigned id;
    in.read((char*)&id, sizeof(id));
    CGE::SceneNode::setIDCount(max(id+1, CGE::SceneNode::getIDCount()));
    //mesh link
    unsigned idx;
    in.read((char*)&idx, sizeof(idx));
    Mesh* msh = meshes_[idx];
    Model* mdl = new Model(msh);
    mdl->setID(id);
    mNodes.push_back(mdl);
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
Model* Scene::pickModel(const Ray& ray) const {
  list<SceneNode*>::const_iterator iter;
  //std::cerr << ray.getOrigin() << " " << ray.getDirection() << "\n";
  Model* nearest = NULL;
  float nearDistance = FLT_MAX;
  for (iter = mNodes.begin(); iter != mNodes.end(); iter++){
    if ((*iter)->getType() != SceneNode::MESH)
      continue;
    Model* mdl = (Model*)*iter;
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
  list<SceneNode*>::iterator iter;
  for (iter = mNodes.begin(); iter != mNodes.end(); iter++){
    if (id == (*iter)->getID()){
      mNodes.erase(iter);
      return;
    }
  }
}

