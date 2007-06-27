
#include "editor.h"
#include "graphics.h"

#include "math/arcball.h"
#include "math/ray.h"
#include "input/keyboard.h"
#include "input/mouse.h"
#include "gui/gui.h"
#include "mesh/model.h"
#include "renderer/renderer.h"

#define TRANSLATION_SPEED 0.1

using Math::Vector2D;
using Math::Vector3D;
using Math::Matrix;
using Math::Ray;
using Gui::Button;
using MeshGeo::Model;

Editor* Editor::edi_ = NULL;

Editor::Editor(){
  arcball_ = new Math::Arcball();
  lastPos_ = Vector2D();
  gridStep_ = 8;
  gridOffset_ = 4;
}

Editor::~Editor(){
  SAFE_DELETE(arcball_);
}

void Editor::init(){
  edi_ = new Editor();
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
  if (key == KEY_R)
    arcball_->update(false,true,Vector2D());
  if (key == KEY_UP){
    Model* mdl = Graphic::instance()->getCurrModel();
    if (!mdl)
      return;
    Matrix oldMat = mdl->getTrafo();
    Vector3D translation = Vector3D(0,0,gridStep_);
    Matrix newMat = Matrix(Matrix::Translation, translation);
    mdl->setTrafo(newMat*oldMat);
  }
  if (key == KEY_DOWN){
    Model* mdl = Graphic::instance()->getCurrModel();
    if (!mdl)
      return;
    Matrix oldMat = mdl->getTrafo();
    Vector3D translation = Vector3D(0,0,-gridStep_);
    Matrix newMat = Matrix(Matrix::Translation, translation);
    mdl->setTrafo(newMat*oldMat);
  }
  if (key == KEY_LEFT){
    Model* mdl = Graphic::instance()->getCurrModel();
    if (!mdl)
      return;
    Matrix oldMat = mdl->getTrafo();
    Vector3D translation = Vector3D(gridStep_,0,0);
    Matrix newMat = Matrix(Matrix::Translation, translation);
    mdl->setTrafo(newMat*oldMat);
  }
  if (key == KEY_RIGHT){
    Model* mdl = Graphic::instance()->getCurrModel();
    if (!mdl)
      return;
    Matrix oldMat = mdl->getTrafo();
    Vector3D translation = Vector3D(-gridStep_,0,0);
    Matrix newMat = Matrix(Matrix::Translation, translation);
    mdl->setTrafo(newMat*oldMat);
  }
}

//! mouse down
void Editor::_mouseDown(int x, int y, int button){
  if (Input::Mouse::instance()->isGuiClick())
    return;
  if (button == MB_MIDDLE){
    lastPos_ = Vector2D(x,y);
  }
  if (button == MB_LEFT){
    //Picking
    Graphics::Renderer* rend = System::Engine::instance()->getRenderer();
    int viewport[4];
    rend->getViewport(viewport);
    rend->projection(60, 4.0f/3.0f, 0.1f, 1000.0f);
    Matrix proj = rend->getMatrix(Graphics::Projection);
    Ray r;
    r.buildPickingRay(x, y, /*viewport[2]-viewport[0], viewport[3]-viewport[1]*/1024, 768, proj.getData()[0], proj.getData()[5], proj.getData()[10]);
    Matrix view = Graphic::instance()->getViewMat();
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
    Graphic::instance()->multCamTrafo(trans);
  }
  
  //Rotation
  bool active = Input::Mouse::instance()->isPressed(MB_RIGHT);
  arcball_->update(active,false,pos);
  Graphic::instance()->setCamRotation(arcball_->getTrafo());
}

