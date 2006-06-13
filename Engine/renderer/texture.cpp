#include <iostream>
#include <fstream>

#include "../system/engine.h"
#include "renderer.h"
#include "texture.h"

using std::ifstream;

using namespace Graphics;

Texture** Texture::loadFromDat(const string& path, const string& filename, int& number){
  ifstream in((path+filename).c_str());
  if (!in){
    System::Log << "Cannot load "+filename;
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
    tex[idx] = System::Engine::instance()->getRenderer()->createTexture(path+name);
  }
  in.close();
  return tex;
}
