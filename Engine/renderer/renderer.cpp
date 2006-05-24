#include "../window/window.h"
#include "../math/vector.h"
#include "renderer.h"

using namespace ::Math;

namespace Renderer{

Renderer::Renderer(){
}

Renderer::~Renderer(){
}

void Renderer::initContext(::Windows::AppWindow* win){
  win_ = win;
}

void Renderer::killContext(){

}

void Renderer::initRendering(){
  setClearColor(Vector3D(0.5,0.5,0));
}

void Renderer::renderScene(){
  clear(ZBUFFER | COLORBUFFER);
}

void Renderer::resizeScene(int width, int height){

}

}

