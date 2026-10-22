#include "system/engine.h"
#include "mesh/mesh.h"
#include "mesh/model.h"
#include "renderer/drawcontext.h"
#include "renderer/renderdevice.h"
#include "renderer/textureloader.h"
#include "renderpass/Forward.h"
#include "renderpass/LightPrepass.h"

#include "editor.h"
#include "menu.h"
#include "graphics.h"

using CGE::Engine;
using CGE::Mesh;
using CGE::Model;
using namespace CGE;

TR_CHANNEL(CGE_Editor);

Graphic* Graphic::gra_ = NULL;

Graphic::Graphic(){
  currModel_ = NULL;
  filename_ = "";

  scene_.setActiveCam(&mCam);
#ifndef RENDER_EMBEDDED
  mTechnique = new CGE::ForwardRenderer();
#else
  mTechnique = new CGE::LightPrepassRenderer();
#endif
  mTechnique->createResources(*Engine::instance()->getDrawContext());
  mTechnique->setScene(scene_);
}

Graphic::~Graphic(){
  mTechnique->releaseResources(*Engine::instance()->getDrawContext());
  delete mTechnique;
}

void Graphic::init(){
  gra_ = new Graphic();

  Editor::instance()->startup();
}

void Graphic::addMesh(std::string filename){
  TR_USE(CGE_Editor);
  CGE::Mesh* mesh = new CGE::Mesh();
  if (!mesh->loadFromFile(filename)){
    TR_ERROR("cannot load file %s", filename.c_str());
    std::cerr << "cannot load file";
    return;
  }
  mesh->buildBuffers(*Engine::instance()->getRenderDevice());
  scene_.addMesh(mesh);
}

void Graphic::render(){
  Editor::instance()->update();
  gra_->render_();
  Menu::render();
}

void Graphic::resize(int width, int height){
  gra_->resize_(width, height);
}

void Graphic::resize_(int width, int height){
}

void Graphic::render_(){
  Gfx::DrawContext& ctx = *Engine::instance()->getDrawContext();
  //the scene with its camera and lights, straight into the window
  Gfx::PassDesc pass;
  pass.clearColor = {0.5f, 0.5f, 0.5f, 1.0f};
  ctx.setColor(1.0,1.0,1.0,1.0);
  ctx.setTexture(nullptr);
  ctx.setBlend(Gfx::BlendMode::Opaque);
  ctx.setCull(Gfx::CullMode::Back);
  mTechnique->render(ctx, pass);
  ctx.setCull(Gfx::CullMode::Off);
  ctx.setDepth(false, false);
}

//! set transformation of a mesh
void Graphic::setTrafo(const int id, const CGE::Matrix& mat){
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
  scene_.addDeviceTexture(Gfx::loadTexture(*Engine::instance()->getRenderDevice(), filename), filename);
}

void Graphic::loadScene(const std::string& filename){
  scene_.load(filename, Engine::instance()->getRenderDevice());
}
