#include <system/engine.h>
#include <renderer/renderer.h>
#include <GL/GL.h>

void initCB(){
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glColor4f(1.0f, 0.0, 0.0, 1.0f);
}

float verts[] = {
  -1.0f, 1.0f,
  -1.0f, -1.0f,
  1.0f, -1.0f
};

void renderCB(){
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 10.0f);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0, 0, -5);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glVertexPointer(2, GL_FLOAT, 0, verts);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
}

void engineMain(int argc, char** argv){
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  rend->setRenderCB(renderCB);
  rend->setInitCB(initCB);
}
