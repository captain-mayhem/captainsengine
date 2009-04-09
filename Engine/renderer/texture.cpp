#include <iostream>
#include <fstream>

#include "../system/engine.h"
#include "renderer.h"
#include "texture.h"

using std::ifstream;

using namespace CGE;

Texture::Texture(std::string filename){
  filename_ = filename;
}

Texture::~Texture(){
}

Texture** Texture::loadFromDat(const string& path, const string& filename, int& number){
  ifstream in((path+filename).c_str());
  if (!in){
    CGE::Log << "Cannot load "+filename;
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
  int n = filename_.find_last_of("\\");
#endif
#ifdef UNIX
  int n = filename_.find_last_of("/");
#endif
  return filename_.substr(n+1);
}

