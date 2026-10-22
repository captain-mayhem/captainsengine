#include <sstream>

#include "system/engine.h"
#include "platform/assets.h"
#include "io/Tracing.h"
#include "renderer/renderdevice.h"
#include "renderer/textureloader.h"
#include "textureManager.h"

TR_CHANNEL(HQ_Textures);

TextureManager* TextureManager::texMan_ = NULL;

TextureManager::TextureManager(){
  loadTextures();
}

TextureManager::~TextureManager(){
}

void TextureManager::loadTextures(){
  loadFromDat("textures/world/","ground.dat",floorTex);
  loadFromDat("textures/world/","wall.dat",wallTex);
  loadFromDat("textures/heros/","heros.dat",heroTex);
  loadFromDat("textures/monsters/","monsters.dat",monsterTex);
  loadFromDat("textures/furniture/","furniture.dat",furnitureTex);
  loadFromDat("textures/overlays/","overlays.dat",overlayTex);
  loadFromDat("textures/other/","other.dat",otherTex);
}

void TextureManager::loadFromDat(const std::string& path, const std::string& filename, Textures& textures){
  TR_USE(HQ_Textures);
  std::string text;
  if (!CGE::Assets::loadText(path+filename, text)){
    TR_WARN("Cannot load %s", filename.c_str());
    return;
  }
  CGE::Gfx::RenderDevice* device = CGE::Engine::instance()->getRenderDevice();
  std::istringstream in(text);
  std::string name;
  int number = 0;
  in >> name;
  in >> number;
  textures.resize(number);
  for (int i = 0; i < number; i++){
    int idx = -1;
    in >> name;
    in >> idx;
    if (idx < 0 || idx >= number){
      TR_WARN("%s: index %d of %s out of range", filename.c_str(), idx, name.c_str());
      continue;
    }
    textures[idx] = CGE::Gfx::loadTexture(*device, path+name);
  }
}
