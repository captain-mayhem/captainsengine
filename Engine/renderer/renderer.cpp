#include "../window/window.h"
#include "../math/vector.h"
#include "renderer.h"
#include "../image/loader.h"
#include "../image/image.h"

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

void Renderer::setMaterial(const Material& mat){
  Color diff = mat.getDiffuse();
  diff.a = mat.getOpacity();
  if (mat.isTransparent())
    enableBlend(true);
  else
    enableBlend(false);
  setColor(&diff);
  Texture const* tex = mat.getDiffuseTex();
  if (!tex)
    tex = mat.getOpacityTex();
  if (tex){
    enableTexturing(true);
    tex->activate();
  }
  else
    enableTexturing(false);
}

Texture* Renderer::createTexture(std::string filename, Texture::Format fmt){
  //filename_ = filename;
  Image *img = NULL;

  img = ImageLoader::load(filename.c_str());

  if (!img)
    return NULL;

  if (fmt == Texture::ALPHA)
    img->convertFormat(1);

  Texture* tex = createTexture(img, fmt);
  if (tex)
    tex->setFilename(filename);

  delete img;

  return tex;
}


