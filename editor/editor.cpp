#define _USE_MATH_DEFINES

#include <math.h>

#include "editor.h"
#include "graphics.h"

#include "math/arcball.h"
#include "math/ray.h"
#include "input/keyboard.h"
#include "input/mouse.h"
#include "mesh/model.h"
#include "system/file.h"

#include <imgui.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define TRANSLATION_SPEED 0.1f

TR_CHANNEL(CGE_Editor_Main);

using CGE::Vector2D;
using CGE::Vector3D;
using CGE::Matrix;
using CGE::Ray;
using CGE::Model;
using CGE::Filesystem;

Editor* Editor::edi_ = nullptr;

Editor::Editor(){
  TR_USE(CGE_Editor_Main);
  arcball_ = new CGE::Arcball();
  lastPos_ = Vector2D();
  gridStep_ = 8;
  gridOffset_ = 4;
  rotationStep_ = (float)M_PI/2.0f;
  editPlane_ = XZ;
  editMode_ = Translation;
  std::string path = Filesystem::getCwd();
  std::cerr << path;
  std::ifstream in((path+"/data/attributes.dat").c_str());
  if (!in){
    TR_ERROR("data/attributes.dat not found!");
    return;
  }
  std::string attrib;
  char buffer[1024];
  while(in >> attrib){
    if (attrib[0] == '#'){
      in.getline(buffer, 1024);
      continue;
    }
    int value;
    in >> value;
    aTov_[attrib] = value;
    vToa_[value] = attrib;
  }
}

Editor::~Editor(){
  SAFE_DELETE(arcball_);
}

void Editor::init(){
  edi_ = new Editor();
  atexit(Editor::release);
}

void Editor::keypress(int key){
  edi_->_keypress(key);
}

void Editor::mouseDown(int x, int y, int button){
  edi_->_mouseDown(x,y,button);
}

void Editor::mouseUp(int x, int y, int button){
  edi_->_mouseUp(x,y,button);
}

void Editor::_keypress(int key){
  if (key == KEY_ESCAPE)
    EXIT();
  //the keys are the view's while ImGui takes the keyboard (e.g. typing a file name)
  if (ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureKeyboard)
    return;
  if (key == KEY_C)
    Graphic::instance()->copyNode(Graphic::instance()->getCurrNode());
  //the selection in view
  if (key == KEY_F)
    Graphic::instance()->frame(Graphic::instance()->getCurrNode());
  if (key == KEY_R)
    editMode_ = Rotation;
  if (key == KEY_T)
    editMode_ = Translation;
    //arcball_->update(false,true,Vector2D());
  if (key == KEY_UP){
    CGE::SceneNode* mdl = Graphic::instance()->getCurrNode();
    if (!mdl)
      return;
    Matrix oldMat = mdl->getTrafo();
    if (editMode_ == Translation){
      Vector3D translation;
      if (editPlane_ == XZ)
        translation = Vector3D(0,0,-gridStep_);
      else if (editPlane_ == XY)
        translation = Vector3D(0,gridStep_,0);
      else if (editPlane_ == YZ)
        translation = Vector3D(0,gridStep_,0);
      Matrix newMat = Matrix(Matrix::Translation, translation);
      mdl->setTrafo(newMat*oldMat);
    }
    else if (editMode_ == Rotation){
      Matrix newMat;
      if (editPlane_ == XZ)
        newMat = Matrix(Matrix::Rotation, Vector3D(0,1,0), rotationStep_);
      else if (editPlane_ == XY)
        newMat = Matrix(Matrix::Rotation, Vector3D(0,0,1), rotationStep_);
      else if (editPlane_ == YZ)
        newMat = Matrix(Matrix::Rotation, Vector3D(1,0,0), rotationStep_);
      mdl->setTrafo(oldMat*newMat);
    }
  }
  if (key == KEY_DOWN){
    CGE::SceneNode* mdl = Graphic::instance()->getCurrNode();
    if (!mdl)
      return;
    Matrix oldMat = mdl->getTrafo();
    if (editMode_ == Translation){
      Vector3D translation;
      if (editPlane_ == XZ)
        translation = Vector3D(0,0,gridStep_);
      else if (editPlane_ == XY)
        translation = Vector3D(0,-gridStep_,0);
      else if (editPlane_ == YZ)
        translation = Vector3D(0,-gridStep_,0);
      Matrix newMat = Matrix(Matrix::Translation, translation);
      mdl->setTrafo(newMat*oldMat);
    }
    else if (editMode_ == Rotation){
      Matrix newMat;
      if (editPlane_ == XZ)
        newMat = Matrix(Matrix::Rotation, Vector3D(0,1,0), -rotationStep_);
      else if (editPlane_ == XY)
        newMat = Matrix(Matrix::Rotation, Vector3D(0,0,1), -rotationStep_);
      else if (editPlane_ == YZ)
        newMat = Matrix(Matrix::Rotation, Vector3D(1,0,0), -rotationStep_);
      mdl->setTrafo(oldMat*newMat);
    }
  }
  if (key == KEY_LEFT){
    CGE::SceneNode* mdl = Graphic::instance()->getCurrNode();
    if (!mdl)
      return;
    Matrix oldMat = mdl->getTrafo();
    if (editMode_ == Translation){
      Vector3D translation;
      if (editPlane_ == XZ)
        translation = Vector3D(-gridStep_,0,0);
      else if (editPlane_ == XY)
        translation = Vector3D(-gridStep_,0,0);
      else if (editPlane_ == YZ)
        translation = Vector3D(0,0,gridStep_);
      Matrix newMat = Matrix(Matrix::Translation, translation);
      mdl->setTrafo(newMat*oldMat);
    }
    else if (editMode_ == Rotation){
      Matrix newMat;
      if (editPlane_ == XZ)
        newMat = Matrix(Matrix::Rotation, Vector3D(0,1,0), rotationStep_);
      else if (editPlane_ == XY)
        newMat = Matrix(Matrix::Rotation, Vector3D(0,0,1), rotationStep_);
      else if (editPlane_ == YZ)
        newMat = Matrix(Matrix::Rotation, Vector3D(1,0,0), rotationStep_);
      mdl->setTrafo(oldMat*newMat);
    }
  }
  if (key == KEY_RIGHT){
    CGE::SceneNode* mdl = Graphic::instance()->getCurrNode();
    if (!mdl)
      return;
    Matrix oldMat = mdl->getTrafo();
    Matrix newMat;
    if (editMode_ == Translation){
      Vector3D translation;
      if (editPlane_ == XZ)
        translation = Vector3D(gridStep_,0,0);
      else if (editPlane_ == XY)
        translation = Vector3D(gridStep_,0,0);
      else if (editPlane_ == YZ)
        translation = Vector3D(0,0,-gridStep_);
      Matrix newMat = Matrix(Matrix::Translation, translation);
      mdl->setTrafo(newMat*oldMat);
    }
    else if (editMode_ == Rotation){
      Matrix newMat;
      if (editPlane_ == XZ)
        newMat = Matrix(Matrix::Rotation, Vector3D(0,1,0), -rotationStep_);
      else if (editPlane_ == XY)
        newMat = Matrix(Matrix::Rotation, Vector3D(0,0,1), -rotationStep_);
      else if (editPlane_ == YZ)
        newMat = Matrix(Matrix::Rotation, Vector3D(1,0,0), -rotationStep_);
      mdl->setTrafo(oldMat*newMat);
    }
  }
  if (key == KEY_DELETE)
    Graphic::instance()->deleteNode(Graphic::instance()->getCurrNode());
}

//! mouse down
void Editor::_mouseDown(int x, int y, int button){
  if (Input::Mouse::instance()->isGuiClick())
    return;
  if (button == MB_MIDDLE){
    lastPos_ = Input::Mouse::instance()->getMousePos();
  }
  if (button == MB_LEFT){
    //Picking
    //the camera's projection in OpenGL's convention, which the ray expects
    const CGE::Frustum& frustum = Graphic::instance()->getCam().getFrustum();
    float top = frustum.getNearDist()*tanf(frustum.getAngle()*(float)M_PI/360.0f);
    float right = top*frustum.getRatio();
    Matrix proj(Matrix::Perspective, -right, right, -top, top, frustum.getNearDist(), frustum.getFarDist());
    //the 3D view covers the whole window, so pick in window pixels
    CGE::VirtualScreen& screen = CGE::Engine::instance()->getVirtualScreen();
    CGE::Vec2i pixel = screen.toWindow(x, y);
    Ray r;
    r.buildPickingRay((float)pixel.x, (float)pixel.y, (float)screen.getWindowWidth(), (float)screen.getWindowHeight(), proj.getData()[0], proj.getData()[5], proj.getData()[10]);
    Matrix view = Graphic::instance()->getCam().getViewMat();
    r.transform(view.inverse());
    //the picked model, or the glTF model it is part of
    Graphic::instance()->setCurrNode(Graphic::instance()->getScene().pickNode(r));
  }
}

//! mouse up
void Editor::_mouseUp(int x, int y, int button){
}

void Editor::update(){
  Vector2D pos = Input::Mouse::instance()->getMousePos();

  EditCam& cam = Graphic::instance()->getCam();
  //Translation
  if (Input::Mouse::instance()->isPressed(MB_MIDDLE)){
    float length = (pos.y-lastPos_.y)*TRANSLATION_SPEED;
    lastPos_ = pos;
    cam.moveForward(-length);
  }
  //the wheel moves a tenth of the way to the point the camera orbits
  if (ImGui::GetCurrentContext() && !ImGui::GetIO().WantCaptureMouse && ImGui::GetIO().MouseWheel != 0)
    cam.moveForward(ImGui::GetIO().MouseWheel*cam.getOrbitDistance()*0.1f);
  
  //Rotation around the point the camera orbits
  bool active = Input::Mouse::instance()->isPressed(MB_RIGHT);
  arcball_->update(active,false,pos);
  cam.orbit(arcball_->getIncTrafo());
}

//! get the string to an attribute
std::string Editor::attribString(int attrib){
  //normal number
  if (attrib < 1000)
    return std::to_string(attrib);
  return vToa_[attrib];
}
  
//! get the attribute to a string
int Editor::attribValue(const std::string& attrib){
  //try to get a value from string
  int att = aTov_[attrib];
  if (att >= 1000)
    return att;
  else
    return toInt(attrib);
}

//! startup after renderer is initialized
void Editor::startup(){
  if (!mLoadScene.empty()){
    Graphic::instance()->loadScene(mLoadScene);
    Graphic::instance()->setFilename(mLoadScene);
  }
  for (unsigned i = 0; i < mLoadModels.size(); ++i){
    if (Graphic::isModelFile(mLoadModels[i])){
      Graphic::instance()->addModelFile(mLoadModels[i]);
      continue;
    }
    size_t count = Graphic::instance()->getScene().getMeshes().size();
    Graphic::instance()->addMesh(mLoadModels[i]);
    if (Graphic::instance()->getScene().getMeshes().size() > count)
      Graphic::instance()->addModel(count);
  }
}