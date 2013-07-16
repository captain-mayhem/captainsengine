#include <system/engine.h>
#include <renderer/renderer.h>
#include <renderer/GL2/GL2Shader.h>
#include <io/Tracing.h>
#include <io/TraceManager.h>
#include <GL/GL.h>

#include "../lib/tristrip.h"
#include "../lib/line.h"

using namespace CGE;

GL2Shader* shader = NULL;
Matrix matrix(Matrix::Identity);
TriStrip strip;
Line line(Vec2f(0.1f, 0.4f), Vec2f(0.9f, 0.7f), 0.04f);

const char* vertexshader = 
"attribute vec3 position;\n"
"uniform mat4 mvp;\n"
"\n"
"void main(){\n"
"  gl_Position = mvp*vec4(position, 1.0);\n"
"}\n";

const char* fragmentshader =
"void main(){\n"
"  gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
"}\n";


void initCB(){
  shader = new GL2Shader;
  shader->addShader(GL_VERTEX_SHADER, vertexshader);
  shader->addShader(GL_FRAGMENT_SHADER, fragmentshader);
  shader->bindAttribLocation(0, "position");
  shader->linkShaders();
  shader->activate();
  glEnableVertexAttribArray(0);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glColor4f(0.0f, 0.0, 0.0, 1.0f);
  Matrix ortho = Matrix(Matrix::Ortho, 0.0f, 1.0f, 0.0f, 1.0f, 0.1f, 10.0f);
  matrix *= ortho;
  Matrix trans = Matrix(Matrix::Translation, Vec3f(0, 0, -5));
  matrix *= trans;
  int loc = shader->getUniformLocation("mvp");
  shader->uniform(loc, matrix);
  /*strip.addVertex(-1.0f, -1.0f);
  strip.addVertex(1.0f, -1.0f);
  strip.addVertex(-1.0f, 1.0f);
  //strip.addVertex(1.0f, 1.0f);*/
  line.convert(strip);
}

void deinitCB(){
  delete shader;
}

void renderCB(){
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, strip.getVertices());
  glDrawArrays(GL_TRIANGLE_STRIP, 0, strip.getNumVertices());
}

void engineMain(int argc, char** argv){
  LogOutputter* putty = new LogOutputter();
  TraceManager::instance()->setTraceOutputter(putty);
  Renderer* rend = Engine::instance()->getRenderer();
  rend->setRenderCB(renderCB);
  rend->setInitCB(initCB);
  rend->setDeinitCB(deinitCB);
}
