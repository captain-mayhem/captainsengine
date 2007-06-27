#include "system/engine.h"
#include "renderer/renderer.h"
#include "renderer/font.h"
#include "mesh/mesh.h"
#include "mesh/model.h"
#include "gui/gui.h"

#include "editor.h"
#include "menu.h"
#include "graphics.h"

using namespace Graphics;
using System::Engine;
using Gui::Button;
using MeshGeo::Mesh;
using MeshGeo::Model;

Graphic* Graphic::gra_ = NULL;

Graphic::Graphic(){
  camTrafo_ = Math::Matrix(Matrix::Translation, Vector3D(0.0,0.0,-10.0f));
  currModel_ = NULL;
}

Graphic::~Graphic(){
  //for (unsigned i = 0; i < meshes_.size(); i++){
  //  delete meshes_[i];
  //}
  //meshes_.clear();
}

void Graphic::init(){
  Renderer* rend = Engine::instance()->getRenderer();
  rend->setClearColor(Vector3D(0.5,0.5,0.5));
  rend->renderMode(Graphics::Filled);
  //Material mat;
  //rend->setMaterial(mat);
  //glEnable(GL_LIGHT0);
  gra_ = new Graphic();

  Button* addTex = new Button();
  addTex->setPosition(Vector2D(900,70));
  addTex->setText("Add texture");
  addTex->setCbFunc(Menu::addTexture);
  System::Engine::instance()->addGuiListener(addTex);
  
  Button* add = new Button();
  add->setPosition(Vector2D(900,100));
  add->setText("Add mesh");
  add->setCbFunc(Menu::addMesh);
  System::Engine::instance()->addGuiListener(add);

  Button* mdl = new Button();
  mdl->setPosition(Vector2D(900,130));
  mdl->setText("Add model");
  mdl->setCbFunc(Menu::addModel);
  System::Engine::instance()->addGuiListener(mdl);

  Button* mdlProp = new Button();
  mdlProp->setPosition(Vector2D(900,160));
  mdlProp->setText("Properties");
  mdlProp->setCbFunc(Menu::properties);
  System::Engine::instance()->addGuiListener(mdlProp);
  
  Button* save = new Button();
  save->setPosition(Vector2D(900,190));
  save->setText("Save scene");
  save->setCbFunc(Menu::save);
  System::Engine::instance()->addGuiListener(save);
  
  Button* load = new Button();
  load->setPosition(Vector2D(900,220));
  load->setText("Load scene");
  load->setCbFunc(Menu::load);
  System::Engine::instance()->addGuiListener(load);
}

void Graphic::addMesh(std::string filename){
  MeshGeo::Mesh* mesh = new MeshGeo::Mesh();
  if (!mesh->loadFromFile(filename)){
    System::Log << "cannot load file";
    std::cerr << "cannot load file";
    return;
  }
  mesh->buildVBO();
  scene_.addMesh(mesh);
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
  rend->multiplyMatrix(camTrafo_);
  rend->multiplyMatrix(camRot_);
  //draw coord sys
  rend->renderMode(Wireframe);
  //x-axis
  //glColor3f(1.0,0.0,0.0);
  //glVertex3f(0,0,0);
  //glVertex3f(10,0,0);
  rend->renderMode(Filled);
  //Font* fnt = System::Engine::instance()->getFont(0);
  //fnt->setColor(0.0,1.0,0.0);
  //fnt->glPrint(200, 400, ("FPS: "+toStr(Engine::instance()->getFPS())).c_str(), 0);
  //rend->enableLighting(true);
  scene_.render();
  //rend->enableLighting(false);
}

//! set transformation of a mesh
void Graphic::setTrafo(const int id, const Math::Matrix& mat){
  scene_.getModel(id)->setTrafo(mat);
  //meshes_[id]->setTrafo(mat);
}

//! add a model to the editor
void Graphic::addModel(unsigned int idx){
  Mesh* mesh = scene_.getMeshes()[idx];
  Model* mdl = new Model(mesh);
  float offset = Editor::instance()->getGridOffset();
  Matrix mat = Matrix(Matrix::Translation, Vector3D(offset,0,offset));
  mdl->setTrafo(mat);
  currModel_ = mdl;
  scene_.addModel(mdl);
}

//! add a texture to the editor
void Graphic::addTexture(const std::string& filename){
  Texture* tex = Texture::create(filename);
  scene_.addTexture(tex);
}

