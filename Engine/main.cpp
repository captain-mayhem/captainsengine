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
    rend->renderMode(Filled);
    rend->projection(45, 1.33333333f,1.0, 1000.0);
    rend->clear(ZBUFFER | COLORBUFFER);
    rend->lookAt(&Vector3D(0,0,-5), &Vector3D(0,0,0), &Vector3D(0,1,0));
    
    VertexBuffer* vb = rend->createVertexBuffer();
    vb->create(VB_POSITION | VB_COLOR, 8, 36);
    vb->lockVertexPointer();
    vb->setPosition(0, Vertex(-1.0, -1.0, -1.0));
    vb->setColor(0, Color(255,0,0,255));
    vb->setPosition(1, Vertex(-1.0, 1.0, -1.0));
    vb->setColor(1, Color(0,255,0,255));
    vb->setPosition(2, Vertex(1.0, 1.0, -1.0));
    vb->setColor(2, Color(0,0,255,255));
    vb->setPosition(3, Vertex(1.0, -1.0, -1.0));
    vb->setColor(3, Color(0,0,0,255));
    vb->setPosition(4, Vertex(-1.0, -1.0, 1.0));
    vb->setColor(4, Color(255,0,0,255));
    vb->setPosition(5, Vertex(-1.0, 1.0, 1.0));
    vb->setColor(5, Color(255,0,0,255));
    vb->setPosition(6, Vertex(1.0, 1.0, 1.0));
    vb->setColor(6, Color(255,0,0,255));
    vb->setPosition(7, Vertex(1.0, -1.0, 1.0));
    vb->setColor(7, Color(255,0,0,255));
    vb->unlockVertexPointer();
    short* indices = vb->lockIndexPointer();
    indices[0] = 0; indices[1] = 1; indices[2] = 2;
    indices[3] = 0; indices[4] = 2; indices[5] = 3;
    indices[6] = 4; indices[7] = 6; indices[8] = 5;
    indices[9] = 4; indices[10] = 7; indices[11] = 6;
    indices[12] = 4; indices[13] = 5; indices[14] = 1;
    indices[15] = 4; indices[16] = 1; indices[17] = 0;
    indices[18] = 3; indices[19] = 2; indices[20] = 6;
    indices[21] = 3; indices[22] = 6; indices[23] = 7;
    indices[24] = 1; indices[25] = 5; indices[26] = 6;
    indices[27] = 1; indices[28] = 6; indices[29] = 2;
    indices[30] = 4; indices[31] = 0; indices[32] = 3;
    indices[33] = 4; indices[34] = 3; indices[35] = 7;
    vb->unlockIndexPointer();
    vb->activate();
    vb->draw();
    delete vb;
  }
};

void engineMain(){
  Rendering* rendering = new Rendering();
  Renderer* rend = Engine::instance()->getRenderer();
  rend->setRenderCB(rendering->render);
  rend->setClearColor(Vector3D(1.0,1.0,1.0));
}