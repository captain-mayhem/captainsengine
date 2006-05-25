#include <iostream>
#include "system/engine.h"
#include "renderer/renderer.h"
#include "math/vector.h"

using std::cerr;
using namespace Math;
using namespace System;
using namespace Graphics;

class Rendering{
public:
  static void render(){
    Renderer* rend = Engine::instance()->getRenderer();
    rend->clear(ZBUFFER | COLORBUFFER);
    VertexBuffer* vb = rend->createVertexBuffer();
    vb->create(VB_POSITION, sizeof(Vertex), 8, 12);
    delete vb;
  }
};

void engineMain(){
  Rendering* rendering = new Rendering();
  Renderer* rend = Engine::instance()->getRenderer();
  rend->setRenderCB(rendering->render);
  rend->setClearColor(Vector3D(0.5,0.5,0));
}