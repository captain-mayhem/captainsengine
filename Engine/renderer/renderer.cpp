#include "../window/window.h"
#include "../math/vector.h"
#include "renderer.h"

using namespace CGE;

Renderer::Renderer(){
  win_ = NULL;
  renderCB_ = NULL;
  initCB_ = NULL;
  deinitCB_ = NULL;
  resizeCB_ = NULL;
}

Renderer::~Renderer(){
}

void Renderer::initContext(AppWindow* win){
  win_ = win;
}

void Renderer::killContext(){

}

void Renderer::initRendering(){
  if (initCB_){
    initCB_();
  }
}

void Renderer::deinitRendering(){
  if (deinitCB_){
    deinitCB_();
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

void Renderer::ortho(const int width, const int height){
  ortho(-width / 2.0f, width / 2.0f, -height / 2.0f, height / 2.0f, -1.f, 1.f);
}


