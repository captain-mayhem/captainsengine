#include "system/engine.h"
#include "renderer/renderer.h"
#include "renderer/font.h"
#include "mesh/mesh.h"
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
  gra_ = new Graphic();
  gra_->addMesh("J:/Projects/medalyvis/simastag/meshes/oelfass.obj");
  gra_->addMesh("/home/captain/medalyvis/simastag/meshes/oelfass.obj");
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
  gra_->render_();
}

void Graphic::render_(){
  Renderer* rend = Engine::instance()->getRenderer(); 
  rend->clear(ZBUFFER | COLORBUFFER);
  rend->setColor(255,255,255,255);
  rend->enableTexturing(false);
  rend->projection(60, 4.0f/3.0f, 0.1f, 1000.0f);
  rend->resetModelView();
  rend->translate(0.0f,0.0f,-10.0f);
  Font* fnt = System::Engine::instance()->getFont(0);
  fnt->setColor(0.0,1.0,0.0);
  fnt->glPrint(200, 400, ("FPS: "+toStr(Engine::instance()->getFPS())).c_str(), 0);
  for (unsigned i = 0; i < meshes_.size(); i++){
    meshes_[i]->draw();
  }
}

