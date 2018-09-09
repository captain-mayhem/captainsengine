#define _USE_MATH_DEFINES

#include <math.h>

#include "editor.h"
#include "graphics.h"

#include "math/arcball.h"
#include "math/ray.h"
#include "input/keyboard.h"
#include "input/mouse.h"
#include "gui/gui.h"
#include "mesh/model.h"
#include "renderer/renderer.h"
#include "system/file.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define TRANSLATION_SPEED 0.1f

TR_CHANNEL(CGE_Editor_Main);

using CGE::Vector2D;
using CGE::Vector3D;
using CGE::Matrix;
using CGE::Ray;
using Gui::Button;
using CGE::Model;
using CGE::Filesystem;

Editor* Editor::edi_ = NULL;

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
  if (key == KEY_C){
    Model* mdl = Graphic::instance()->getCurrModel();
    if (!mdl)
      return;
    Model* copy = new Model(*mdl);
    Graphic::instance()->getScene().addModel(copy);
  }
  if (key == KEY_R)
    editMode_ = Rotation;
  if (key == KEY_T)
    editMode_ = Translation;
    //arcball_->update(false,true,Vector2D());
  if (key == KEY_UP){
    Model* mdl = Graphic::instance()->getCurrModel();
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
    Model* mdl = Graphic::instance()->getCurrModel();
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
    Model* mdl = Graphic::instance()->getCurrModel();
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
    Model* mdl = Graphic::instance()->getCurrModel();
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
  if (key == KEY_DELETE){
    Model* mdl = Graphic::instance()->getCurrModel();
    if (!mdl)
      return;
    Graphic::instance()->getScene().deleteModel(mdl->getID());
  }
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
    CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
    int viewport[4];
    rend->getViewport(viewport);
    rend->projection(60, 4.0f/3.0f, 0.1f, 1000.0f);
    Matrix proj = rend->getMatrix(CGE::Projection);
    Ray r;
    r.buildPickingRay((float)x, (float)y, /*viewport[2]-viewport[0], viewport[3]-viewport[1]*/1024.0f, 768.0f, proj.getData()[0], proj.getData()[5], proj.getData()[10]);
    Matrix view = Graphic::instance()->getCam().getViewMat();
    r.transform(view.inverse());
    //get the picked model
    Model* mdl = Graphic::instance()->getScene().pickModel(r);
    Graphic::instance()->setCurrModel(mdl);
  }
}

//! mouse up
void Editor::_mouseUp(int x, int y, int button){
}

void Editor::update(){
  Vector2D pos = Input::Mouse::instance()->getMousePos();

  //Translation
  if (Input::Mouse::instance()->isPressed(MB_MIDDLE)){
    Vector3D dir = Vector3D(0,0,-1);
    float length = (pos.y-lastPos_.y)*TRANSLATION_SPEED;
    lastPos_ = pos;
    Matrix trans = Matrix(Matrix::Translation, dir*length);
    Graphic::instance()->getCam().multCamTrafo(trans);
  }
  
  //Rotation
  bool active = Input::Mouse::instance()->isPressed(MB_RIGHT);
  arcball_->update(active,false,pos);
  //Graphic::instance()->setCamRotation(arcball_->getTrafo());
  Graphic::instance()->getCam().multCamTrafo(arcball_->getIncTrafo());
}

//! get the string to an attribute
std::string Editor::attribString(int attrib){
  //normal number
  if (attrib < 1000)
    return toStr(attrib);
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
    Graphic::instance()->getScene().load(mLoadScene);
    Graphic::instance()->setFilename(mLoadScene);
  }
  for (unsigned i = 0; i < mLoadModels.size(); ++i){
    Graphic::instance()->addMesh(mLoadModels[i]);
    Graphic::instance()->addModel(Graphic::instance()->getScene().getMeshes().size()-1);
  }
}