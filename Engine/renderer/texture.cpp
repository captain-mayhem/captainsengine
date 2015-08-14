#include <iostream>
#include <fstream>

#include "../system/engine.h"
#include "renderer.h"
#include "texture.h"
#include "../image/loader.h"

using std::ifstream;

using namespace CGE;

TR_CHANNEL(CGE_Texture);

Texture::Texture(Format fmt) : mFormat(fmt){
}

Texture::~Texture(){
}

Texture** Texture::loadFromDat(const string& path, const string& filename, int& number){
  TR_USE(CGE_Texture);
  ifstream in((path+filename).c_str());
  if (!in){
    TR_WARN("Cannot load %s", filename.c_str());
    return NULL;
  }
  string name;
  int idx;
  in >> name;
  in >> number;
  Texture** tex = new Texture*[number];
  for (int i = 0; i < number; i++){
    in >> name;
    in >> idx;
    tex[idx] = CGE::Engine::instance()->getRenderer()->createTexture(path+name);
  }
  in.close();
  return tex;
}

//! load a texture
Texture* Texture::create(const std::string& filename){
  return CGE::Engine::instance()->getRenderer()->createTexture(filename);
}

//! get the name
std::string Texture::getName() const{
#ifdef WIN32
  size_t n = filename_.find_last_of("\\");
#endif
#ifdef UNIX
  size_t n = filename_.find_last_of("/");
#endif
  return filename_.substr(n+1);
}

