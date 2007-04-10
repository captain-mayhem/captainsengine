//#include <GL/gl.h>
#include "system/engine.h"
#include "renderer/renderer.h"
#include "renderer/font.h"
#include "mesh/mesh.h"

#include "editor.h"
#include "graphics.h"

using namespace Graphics;
using System::Engine;

Graphic* Graphic::gra_ = NULL;

Graphic::Graphic(){
}

Graphic::~Graphic(){
  for (unsigned i = 0; i < meshes_.size(); i++){
    delete meshes_[i];
  }
  meshes_.clear();
}

void Graphic::init(){
  Renderer* rend = Engine::instance()->getRenderer();
  rend->setClearColor(Vector3D(0.5,0.5,0.5));
  rend->renderMode(Graphics::Filled);
  Material mat;
  //rend->setMaterial(mat);
  //glEnable(GL_LIGHT0);
  gra_ = new Graphic();
  gra_->addMesh("../heroquest/HQClient/models/world/pit2.obj");
  //gra_->addMesh("ground.obj");
}

void Graphic::addMesh(std::string filename){
  MeshGeo::Mesh* mesh = new MeshGeo::Mesh();
  if (!mesh->loadFromFile(filename)){
    System::Log << "cannot load file";
    return;
  }
  mesh->buildVBO();
  meshes_.push_back(mesh);
}

void Graphic::render(){
  Editor::instance()->update();
  gra_->render_();
}

void Graphic::render_(){
  Renderer* rend = Engine::instance()->getRenderer(); 
  rend->clear(ZBUFFER | COLORBUFFER);
  rend->setColor(1.0,1.0,1.0,1.0);
  rend->enableTexturing(false);
  rend->projection(60, 4.0f/3.0f, 0.1f, 1000.0f);
  rend->resetModelView();
  rend->translate(0.0f,0.0f,-10.0f);
  //rend->setMatrix(trafo_);
  Font* fnt = System::Engine::instance()->getFont(0);
  fnt->setColor(0.0,1.0,0.0);
  fnt->glPrint(200, 400, ("FPS: "+toStr(Engine::instance()->getFPS())).c_str(), 0);
  //rend->enableLighting(true);
  for (unsigned i = 0; i < meshes_.size(); i++){
    meshes_[i]->draw();
  }
  //rend->enableLighting(false);
}

//! set transformation of a mesh
void Graphic::setTrafo(const int id, const Math::Matrix& mat){
  meshes_[id]->setTrafo(mat);
}
