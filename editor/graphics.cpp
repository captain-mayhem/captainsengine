#include <algorithm>
#include <cctype>
#include <cfloat>
#include <cmath>

#include "system/engine.h"
#include "mesh/mesh.h"
#include "mesh/model.h"
#include "mesh/modelfile.h"
#include "renderer/light.h"
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
  currNode_ = NULL;
  mDefaultLights = true;
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
  CGE::VirtualScreen& screen = Engine::instance()->getVirtualScreen();
  gra_->resize_(screen.getWindowWidth(), screen.getWindowHeight());

  Editor::instance()->startup();
}

void Graphic::addMesh(std::string filename){
  TR_USE(CGE_Editor);
  CGE::Mesh* mesh = new CGE::Mesh();
  if (!mesh->loadFromFile(filename)){
    TR_ERROR("cannot load file %s", filename.c_str());
    std::cerr << "cannot load file";
    mMessage = "Cannot load "+filename;
    delete mesh;
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
  //the view keeps the window's shape
  if (width <= 0 || height <= 0)
    return;
  const Frustum& frustum = mCam.getFrustum();
  mCam.project(frustum.getAngle(), (float)width/(float)height, frustum.getNearDist(), frustum.getFarDist());
}

void Graphic::render_(){
  Gfx::DrawContext& ctx = *Engine::instance()->getDrawContext();
  //a scene without lights would be drawn unlit: a key and a fill light
  //from the camera's side, turning with it
  std::vector<Light*>& lights = scene_.getLights();
  bool defaultLights = mDefaultLights && lights.empty();
  Light key(Light::Directional, Vec3f());
  Light fill(Light::Directional, Vec3f());
  if (defaultLights){
    Matrix inv = mCam.getViewMat().inverse();
    Vec3f side = inv*Vec4f(1, 0, 0, 0);
    Vec3f up = inv*Vec4f(0, 1, 0, 0);
    Vec3f back = inv*Vec4f(0, 0, 1, 0);
    Vec3f keyDir = side*0.5f+up*0.8f+back;
    Vec3f fillDir = side*-0.8f+up*0.1f+back*0.6f;
    key.setPosition(Vec4f(keyDir.x, keyDir.y, keyDir.z, 0));
    fill.setPosition(Vec4f(fillDir.x, fillDir.y, fillDir.z, 0));
    fill.setColor(Color(0.45f, 0.45f, 0.5f, 1));
    lights.push_back(&key);
    lights.push_back(&fill);
  }
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
  if (defaultLights)
    lights.clear();
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
  currNode_ = mdl;
  scene_.addModel(mdl);
}

CGE::Model* Graphic::getCurrModel() const{
  if (!currNode_ || currNode_->getType() != SceneNode::MESH)
    return nullptr;
  return (Model*)currNode_;
}

bool Graphic::isModelFile(const std::string& filename){
  size_t dot = filename.find_last_of('.');
  if (dot == std::string::npos)
    return false;
  std::string ext = filename.substr(dot+1);
  std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){return (char)std::tolower(c);});
  return ext == "gltf" || ext == "glb";
}

bool Graphic::addModelFile(const std::string& filename){
  TR_USE(CGE_Editor);
  std::string error;
  std::unique_ptr<ModelFile> model = ModelFile::load(filename, &error);
  if (!model){
    TR_ERROR("cannot load %s: %s", filename.c_str(), error.c_str());
    mMessage = "Cannot load "+filename+": "+error;
    return false;
  }
  model->buildBuffers(*Engine::instance()->getRenderDevice());
  mMessage.clear();
  if (!model->getWarnings().empty())
    mMessage = std::to_string(model->getWarnings().size())+" warnings for "+filename+" (see Model)";
  //the root has no name; the file's name tells the copies apart
  std::string name = filename.substr(filename.find_last_of("/\\")+1);
  model->getRoot()->setName(name);
  mModelFiles.push_back(std::move(model));
  addModelFileInstance((unsigned)mModelFiles.size()-1);
  frame(currNode_);
  return true;
}

void Graphic::addModelFileInstance(unsigned idx){
  if (idx >= mModelFiles.size())
    return;
  ModelFile* model = mModelFiles[idx].get();
  SceneNode* node = model->getRoot()->clone();
  node->setName(model->getRoot()->getName());
  scene_.addNode(node);
  mModelFileNodes[node] = model;
  currNode_ = node;
}

CGE::ModelFile* Graphic::getModelFileOf(CGE::SceneNode* node) const{
  auto found = mModelFileNodes.find(node);
  return found == mModelFileNodes.end() ? nullptr : found->second;
}

void Graphic::copyNode(CGE::SceneNode* node){
  if (!node)
    return;
  SceneNode* copy = node->clone();
  copy->setName(node->getName());
  scene_.addNode(copy);
  if (ModelFile* model = getModelFileOf(node))
    mModelFileNodes[copy] = model;
  currNode_ = copy;
}

void Graphic::deleteNode(CGE::SceneNode* node){
  if (!node)
    return;
  scene_.deleteModel(node->getID());
  mModelFileNodes.erase(node);
  if (currNode_ == node)
    currNode_ = nullptr;
  delete node;
}

namespace{

//! widens box by the meshes' boxes at and below node, in trafo
void addBounds(const SceneNode* node, const Matrix& parent, Vec3f& lower, Vec3f& upper){
  Matrix trafo = parent*node->getTrafo();
  if (node->getType() == SceneNode::MESH && ((const Model*)node)->getMesh()){
    BBox box = ((const Model*)node)->getMesh()->getExtents();
    Vec3f lo = box.getMin();
    Vec3f hi = box.getMax();
    for (int i = 0; i < 8; ++i){
      Vec3f corner(i & 1 ? hi.x : lo.x, i & 2 ? hi.y : lo.y, i & 4 ? hi.z : lo.z);
      Vec3f p = trafo*corner;
      lower = Vec3f((std::min)(lower.x, p.x), (std::min)(lower.y, p.y), (std::min)(lower.z, p.z));
      upper = Vec3f((std::max)(upper.x, p.x), (std::max)(upper.y, p.y), (std::max)(upper.z, p.z));
    }
  }
  for (const SceneNode* child : node->getChildren())
    addBounds(child, trafo, lower, upper);
}

}

void Graphic::frame(CGE::SceneNode* node){
  if (!node)
    return;
  Vec3f lower(FLT_MAX, FLT_MAX, FLT_MAX);
  Vec3f upper(-FLT_MAX, -FLT_MAX, -FLT_MAX);
  addBounds(node, Matrix(Matrix::Identity), lower, upper);
  if (lower.x > upper.x)
    return;
  Vec3f center = (lower+upper)*0.5f;
  float radius = (upper-lower).length()*0.5f;
  if (radius < 0.001f)
    radius = 0.001f;
  //the bounding sphere fits the narrower side of the view
  const Frustum& frustum = mCam.getFrustum();
  float half = frustum.getAngle()*0.5f*3.14159265f/180.0f;
  if (frustum.getRatio() < 1.0f)
    half = atanf(tanf(half)*frustum.getRatio());
  mCam.lookAtFrom(center, radius/sinf(half)*1.1f);
}

//! add a texture to the editor
void Graphic::addTexture(const std::string& filename){
  scene_.addDeviceTexture(Gfx::loadTexture(*Engine::instance()->getRenderDevice(), filename), filename);
}

void Graphic::loadScene(const std::string& filename){
  scene_.load(filename, *Engine::instance()->getRenderDevice());
}
