#include "system/engine.h"
#include "renderer/renderer.h"
#include "renderer/font.h"
#include "graphics.h"

using namespace Graphics;
using System::Engine;

Graphic::Graphic(){
}

Graphic::~Graphic(){
}

void Graphic::init(){
  Renderer* rend = Engine::instance()->getRenderer();
  rend->setClearColor(Vector3D(0.5,0.5,0.5));
  rend->renderMode(Graphics::Filled);
}

void Graphic::render(){
  Renderer* rend = Engine::instance()->getRenderer(); 
  rend->clear(ZBUFFER | COLORBUFFER);
  rend->setColor(255,255,255,255);
  Font* fnt = System::Engine::instance()->getFont(0);
  fnt->setColor(0.0,1.0,0.0);
  fnt->glPrint(200, 400, ("FPS: "+toStr(Engine::instance()->getFPS())).c_str(), 0);
}

