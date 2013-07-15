#include <system/engine.h>
#include <renderer/renderer.h>
#include <GL/GL.h>

#include "../lib/tristrip.h"
#include "../lib/line.h"

using namespace CGE;

TriStrip strip;
Line line(Vec2f(0.1f, 0.4f), Vec2f(0.9f, 0.7f), 0.04f);

void initCB(){
  glEnableClientState(GL_VERTEX_ARRAY);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glColor4f(0.0f, 0.0, 0.0, 1.0f);
  /*strip.addVertex(-1.0f, -1.0f);
  strip.addVertex(1.0f, -1.0f);
  strip.addVertex(-1.0f, 1.0f);
  //strip.addVertex(1.0f, 1.0f);*/
  line.convert(strip);
}

void renderCB(){
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0f, 1.0f, 0.0f, 1.0f, 0.1f, 10.0f);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0, 0, -5);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glVertexPointer(3, GL_FLOAT, 0, strip.getVertices());
  glDrawArrays(GL_TRIANGLE_STRIP, 0, strip.getNumVertices());
}

void engineMain(int argc, char** argv){
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  rend->setRenderCB(renderCB);
  rend->setInitCB(initCB);
}
