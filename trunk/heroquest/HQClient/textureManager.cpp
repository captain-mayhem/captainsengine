#include "system/engine.h"
#include "renderer/texture.h"
#include "textureManager.h"

TextureManager* TextureManager::texMan_ = NULL;

TextureManager::TextureManager(){
  floorTex = NULL;
  wallTex = NULL;
  heroTex = NULL;
  monsterTex = NULL;
  furnitureTex = NULL;
  overlayTex = NULL;
  otherTex = NULL;
  loadTextures();
}

TextureManager::~TextureManager(){
  for (int i = 0; i < floorNum_; i++){
    SAFE_DELETE(floorTex[i]);
  }
  SAFE_DELETE_ARRAY(floorTex);
  for (int i = 0; i < wallNum_; i++){
    SAFE_DELETE(wallTex[i]);
  }
  SAFE_DELETE_ARRAY(wallTex);
  for (int i = 0; i < heroNum_; i++){
    SAFE_DELETE(heroTex[i]);
  }
  SAFE_DELETE_ARRAY(heroTex);
  for (int i = 0; i < monsterNum_; i++){
    SAFE_DELETE(monsterTex[i]);
  }
  SAFE_DELETE_ARRAY(monsterTex);
  for (int i = 0; i < furnitureNum_; i++){
    SAFE_DELETE(furnitureTex[i]);
  }
  SAFE_DELETE_ARRAY(furnitureTex);
  for (int i = 0; i < overlayNum_; i++){
    SAFE_DELETE(overlayTex[i]);
  }
  SAFE_DELETE_ARRAY(overlayTex);
  for (int i = 0; i < otherNum_; i++){
    SAFE_DELETE(otherTex[i]);
  }
  SAFE_DELETE_ARRAY(otherTex);
}

void TextureManager::loadTextures(){
  floorTex = CGE::Texture::loadFromDat("textures/world/","ground.dat",floorNum_);
  wallTex = CGE::Texture::loadFromDat("textures/world/","wall.dat",wallNum_);
  heroTex = CGE::Texture::loadFromDat("textures/heros/","heros.dat",heroNum_);
  monsterTex = CGE::Texture::loadFromDat("textures/monsters/","monsters.dat",monsterNum_);
  furnitureTex = CGE::Texture::loadFromDat("textures/furniture/","furniture.dat",furnitureNum_);
  overlayTex = CGE::Texture::loadFromDat("textures/overlays/","overlays.dat",overlayNum_);
  otherTex = CGE::Texture::loadFromDat("textures/other/","other.dat",otherNum_);
}
