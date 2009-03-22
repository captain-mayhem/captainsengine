
#include "Vehicle.h"

#define _USE_MATH_DEFINES
#include "math.h"
#include "renderer/renderer.h"
#include "renderer/forms.h"
#include "system/engine.h"

void Vehicle::render(const Graphics::Camera& cam){
  Graphics::Renderer* rend = System::Engine::instance()->getRenderer();
  Graphics::Forms* form = System::Engine::instance()->getForms();
  rend->setColor(1.0,0.0,0.0,1.0);
  form->activateCylinder();
  Math::Matrix rot = Math::Matrix(Matrix::Rotation, Vec3f(1,0,0), M_PI/2);
  //rot = rot.inverse();
  rend->multiplyMatrix(rot);
  rend->translate(2,2,0);
  form->drawCylinder();
  rend->translate(0,2,0);
  form->drawCylinder();
  rend->translate(4,0,0);
  form->drawCylinder();
  rend->translate(0,-2,0);
  form->drawCylinder();
}