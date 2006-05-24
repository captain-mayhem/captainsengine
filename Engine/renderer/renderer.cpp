#include "../window/window.h"
#include "../math/vector.h"
#include "renderer.h"

using namespace ::Math;

namespace Graphics{

Renderer::Renderer(){
  win_ = NULL;
  renderCB_ = NULL;
}

Renderer::~Renderer(){
}

void Renderer::initContext(::Windows::AppWindow* win){
  win_ = win;
}

void Renderer::killContext(){

}

void Renderer::initRendering(){
  
}

void Renderer::renderScene(){
  if (renderCB_){
    renderCB_();
  }
}

void Renderer::resizeScene(int width, int height){

}

}

