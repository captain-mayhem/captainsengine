#include "system/engine.h"
#include "renderer/renderer.h"
#include "renderer/font.h"
#include "mesh/mesh.h"
#include "mesh/model.h"
#include "gui/gui.h"
#include "window/window.h"
#include "renderer/forms.h"
#include "renderpass/Forward.h"
#include "renderpass/LightPrepass.h"

#include "editor.h"
#include "menu.h"
#include "graphics.h"

using CGE::Engine;
using Gui::Button;
using CGE::Mesh;
using CGE::Model;
using namespace CGE;

TR_CHANNEL(CGE_Editor);

Graphic* Graphic::gra_ = NULL;

Graphic::Graphic() : mRT(NULL){
  currModel_ = NULL;
  filename_ = "";

  Renderer* rend = Engine::instance()->getRenderer();
  rend->enableBackFaceCulling(true);
  mRT = rend->createRenderTarget(rend->getWindow()->getWidth(), rend->getWindow()->getHeight());
  if (mRT){
    mRT->activate();
    mRT->addTexture(CGE::Texture::RGBA);
    mRT->addRenderbuffer(CGE::Texture::DEPTH);
    mRT->isComplete();
    mRT->deactivate();
  }
  scene_.setActiveCam(&mCam);
#if 1
  mTechnique = new CGE::ForwardRenderer();
#else
  mTechnique = new CGE::LightPrepassRenderer();
#endif
  mTechnique->init();
  mTechnique->setScene(scene_);
}

Graphic::~Graphic(){
  mTechnique->deinit();
  delete mTechnique;
  delete mRT;
}

void Graphic::init(){
  Renderer* rend = Engine::instance()->getRenderer();
  rend->setClearColor(Vector3D(0.5,0.5,0.5));
  rend->renderMode(CGE::Filled);

  rend->enableLighting(true);
  rend->setNumLights(2);
  rend->enableLighting(false);
  //Material mat;
  //rend->setMaterial(mat);
  //glEnable(GL_LIGHT0);
  gra_ = new Graphic();
  atexit(Graphic::release);

  Button* opt = new Button();
  opt->setPosition(Vector2D(900,40));
  opt->setText("Options");
  opt->setCbFunc(Menu::options);
  CGE::Engine::instance()->addGuiListener(opt);

  Button* addTex = new Button();
  addTex->setPosition(Vector2D(900,70));
  addTex->setText("Add texture");
  addTex->setCbFunc(Menu::addTexture);
  CGE::Engine::instance()->addGuiListener(addTex);
  
  Button* add = new Button();
  add->setPosition(Vector2D(900,100));
  add->setText("Add mesh");
  add->setCbFunc(Menu::addMesh);
  CGE::Engine::instance()->addGuiListener(add);

  Button* mdl = new Button();
  mdl->setPosition(Vector2D(900,130));
  mdl->setText("Add model");
  mdl->setCbFunc(Menu::addModel);
  CGE::Engine::instance()->addGuiListener(mdl);

  Button* mdlProp = new Button();
  mdlProp->setPosition(Vector2D(900,160));
  mdlProp->setText("Properties");
  mdlProp->setCbFunc(Menu::properties);
  CGE::Engine::instance()->addGuiListener(mdlProp);
  
  Button* save = new Button();
  save->setPosition(Vector2D(900,190));
  save->setText("Save scene");
  save->setCbFunc(Menu::save);
  CGE::Engine::instance()->addGuiListener(save);
  
  Button* load = new Button();
  load->setPosition(Vector2D(900,220));
  load->setText("Load scene");
  load->setCbFunc(Menu::load);
  CGE::Engine::instance()->addGuiListener(load);

  Button* hqm = new Button();
  hqm->setPosition(Vector2D(900, 250));
  hqm->setText("Export HQM");
  hqm->setCbFunc(Menu::hqmExport);
  CGE::Engine::instance()->addGuiListener(hqm);

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
  mesh->buildVBO();
  scene_.addMesh(mesh);
}

void Graphic::render(){
  Editor::instance()->update();
  gra_->render_();
}

void Graphic::resize(int width, int height){
  gra_->resize_(width, height);
}

void Graphic::resize_(int width, int height){
  Renderer* rend = Engine::instance()->getRenderer();
  //delete mRT;
  //mRT = rend->createRenderTarget(width, height);
}

void Graphic::render_(){
  Renderer* rend = Engine::instance()->getRenderer();
  if (mRT)
    mRT->activate();
  rend->enableLighting(true);
  rend->clear(ZBUFFER | COLORBUFFER);
  rend->setColor(1.0,1.0,1.0,1.0);
  rend->enableTexturing(false);
  
  //rend->projection(60, 4.0f/3.0f, 0.01f, 1000.0f);
  //rend->ortho(-50, 50, -50, 50, 0.1, 100);
  rend->resetModelView();

  CGE::Light lt(CGE::Light::Point, Vec3f(0, 0.5, 0.5));
  CGE::Light lt2(Vec3f(0, 0.5, 0.5), Vec3f(0, 0, -1), 30);
  rend->setLight(1, lt);
  rend->setLight(0, lt2);

  mCam.activate();
  //rend->multiplyMatrix(mCam.getViewMat());
  //draw coord sys
  rend->renderMode(Wireframe);
  //x-axis
  //glColor3f(1.0,0.0,0.0);
  //glVertex3f(0,0,0);
  //glVertex3f(10,0,0);
  rend->renderMode(Filled);
  //Font* fnt = CGE::Engine::instance()->getFont(0);
  //fnt->setColor(0.0,1.0,0.0);
  //fnt->glPrint(200, 400, ("FPS: "+toStr(Engine::instance()->getFPS())).c_str(), 0);
  rend->enableTexturing(true);
  rend->blendFunc(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
  mTechnique->render();
  rend->enableDepthWrite(true);
  rend->enableLighting(false);
  if (mRT){
    mRT->deactivate();

    rend->ortho(1, 1);
    rend->resetModelView();
    rend->enableDepthTest(false);
    CGE::Forms& forms = *CGE::Engine::instance()->getForms();
    forms.activateQuad();
    rend->enableTexturing(true);
    rend->setColor(1, 1, 1, 1);
    if (rend->getRenderType() != DirectX){
      rend->switchMatrixStack(CGE::MatTexture);
      rend->resetModelView();
      rend->scale(1, -1, 1);
    }
    mRT->getTexture(0)->activate();
    forms.drawQuad(Vec2f(), Vec2f(1, 1));
    if (rend->getRenderType() != DirectX){
      rend->resetModelView();
      rend->switchMatrixStack(CGE::Modelview);
    }
  }
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
  Texture* tex = Texture::create(filename);
  scene_.addTexture(tex);
}

