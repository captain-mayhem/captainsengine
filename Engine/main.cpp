#include <iostream>
#include "system/engine.h"
#include "renderer/renderer.h"
#include "renderer/font.h"
#include "math/vector.h"

using std::cerr;
using namespace Math;
using namespace System;
using namespace Graphics;

class Rendering{
private:
    static VertexBuffer* vb;
    static Texture* tex;
    static Font* font;
public:
  Rendering(){}
  ~Rendering(){
    delete vb;
    delete tex;
    delete font;
  }
  static void init(){
    Renderer* rend = Engine::instance()->getRenderer();
    rend->setClearColor(Vector3D(0.5,0.5,0.5));
    rend->renderMode(Filled);
    vb = rend->createVertexBuffer();
    tex = rend->createTexture("textures/font/font.bmp");
    font = new Font();
    font->buildFont();
    
    vb->create(VB_POSITION | VB_COLOR | VB_TEXCOORD, 8, 36);
    vb->lockVertexPointer();
    vb->setPosition(0, Vertex(-1.0, -1.0, -1.0));
    vb->setColor(0, Color(255,0,0,255));
    vb->setTexCoord(0, Vec2f(0,0));
    vb->setPosition(1, Vertex(-1.0, 1.0, -1.0));
    vb->setColor(1, Color(0,255,0,255));
    vb->setTexCoord(1, Vec2f(0,1));
    vb->setPosition(2, Vertex(1.0, 1.0, -1.0));
    vb->setColor(2, Color(0,0,255,255));
    vb->setTexCoord(2, Vec2f(1,1));
    vb->setPosition(3, Vertex(1.0, -1.0, -1.0));
    vb->setColor(3, Color(0,0,0,255));
    vb->setTexCoord(3, Vec2f(1,0));
    vb->setPosition(4, Vertex(-1.0, -1.0, 1.0));
    vb->setColor(4, Color(255,0,0,255));
    vb->setTexCoord(4, Vec2f(0,0));
    vb->setPosition(5, Vertex(-1.0, 1.0, 1.0));
    vb->setColor(5, Color(255,0,0,255));
    vb->setTexCoord(5, Vec2f(0,1));
    vb->setPosition(6, Vertex(1.0, 1.0, 1.0));
    vb->setColor(6, Color(255,0,0,255));
    vb->setTexCoord(6, Vec2f(1,1));
    vb->setPosition(7, Vertex(1.0, -1.0, 1.0));
    vb->setColor(7, Color(255,0,0,255));
    vb->setTexCoord(7, Vec2f(1,-1));
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
  }

  static void render(){
    Renderer* rend = Engine::instance()->getRenderer();
    
    rend->projection(45, 1.33333333f,1.0, 1000.0);
    //rend->ortho();
    rend->clear(ZBUFFER | COLORBUFFER);
    rend->setColor(255,255,255,255);
    rend->resetModelView();
    static float l = -5;
    //l += 0.0001;
    rend->lookAt(Vector3D(0,0,5), Vector3D(0,0,0), Vector3D(0,1,0));
    
    tex->activate();
    vb->activate();
    vb->draw(VB_Triangles);

    rend->ortho(1024, 768);
    rend->resetModelView();
    rend->translate(-512, -384, 0);
    rend->blendFunc(BLEND_SRC_ALPHA, BLEND_ONE);
    rend->enableBlend(true);
    font->setColor(0.0,1.0,0);
    //static int count = 0;
    //count++;
    //if (count == 100){
      font->glPrint(200, 400, "FPS: "+toStr(Engine::instance()->getFPS()), 0);
    //  count = 0;
    //}
    font->render();
    rend->enableBlend(false);
  }
};

VertexBuffer* Rendering::vb = NULL;
Texture* Rendering::tex = NULL;
Font* Rendering::font = NULL;

void engineMain(){
  Rendering* rendering = new Rendering();
  Renderer* rend = Engine::instance()->getRenderer();
  rend->setRenderCB(rendering->render);
  rend->setInitCB(rendering->init);
}
