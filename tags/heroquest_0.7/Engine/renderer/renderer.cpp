#include "../window/window.h"
#include "../math/vector.h"
#include "renderer.h"

using namespace ::Math;

using namespace Graphics;

Renderer::Renderer(){
  win_ = NULL;
  renderCB_ = NULL;
  initCB_ = NULL;
  resizeCB_ = NULL;
}

Renderer::~Renderer(){
}

void Renderer::initContext(::Windows::AppWindow* win){
  win_ = win;
}

void Renderer::killContext(){

}

void Renderer::initRendering(){
  if (initCB_){
    initCB_();
  }
  
}

void Renderer::renderScene(){
  if (renderCB_){
    renderCB_();
  }
}

void Renderer::resizeScene(int width, int height){
  if (resizeCB_){
    resizeCB_(width, height);
  }
}


