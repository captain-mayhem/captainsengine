#include "material.h"
#include <system/file.h>
#include <system/engine.h>
#include <renderer/renderer.h>

using namespace CGE;

Material::Material(std::string const& name) : mName(name), mDiffuseTex(NULL){
  mAmbient = Color(0.0f,0.0f,0.0f,1.0f);
  mDiffuse = Color(1.0f,1.0f,1.0f,1.0f);
  mSpecular = Color(0.0f,0.0f,0.0f,1.0f);
  mPower = 0.0f;
}

Material::~Material(){
}

bool Material::loadFromMTL(std::string const& filename, std::vector<Material*>& materials, std::vector<Texture*>& textures){
  FILE* f = fopen(filename.c_str(), "r");
  if (!f)
    return false;

  char line[2000];
  char* pos;
  
  Material* currMat = NULL;
  Color col;

  std::map<std::string, Texture*> texturemap;
  for (unsigned i = 0; i < textures.size(); ++i){
    Texture* tex = textures[i];
    texturemap[tex->getName()] = tex;
  }

  while (fgets(line, 2000, f)){
    switch (line[0]){
    case 'm':
      if (memcmp(line, "map_Kd", 6) == 0){
        std::string texture(line + 7);
        texture.pop_back();
        std::string path = Filesystem::getPathComponent(filename);
        texture = Filesystem::combinePath(path, texture);
        Texture* tex = texturemap[texture];
        if (tex == NULL){
          tex = Engine::instance()->getRenderer()->createTexture(texture);
          texturemap[texture] = tex;
          if (tex)
            textures.push_back(tex);
        }
        currMat->setDiffuseTex(tex);
      }
      break;
    case 'n':
      if (memcmp(line, "newmtl", 6) == 0){
        std::string mtl(line + 7);
        mtl.pop_back();
        if (currMat != NULL)
          materials.push_back(currMat);
        currMat = new Material(mtl);
      }
      break;
    case 'N':
      if (line[1] == 's'){
        float exp = strtof(line+3, NULL);
        currMat->setPower(exp);
      }
      break;
    case 'K':
      col.r = strtof(line + 3, &pos);
      col.g = strtof(pos+1, &pos);
      col.b = strtof(pos+1, &pos);
      if (line[1] == 'd')
        currMat->setDiffuse(col);
      else if (line[1] == 'a')
        currMat->setAmbient(col);
      else if (line[1] == 's')
        currMat->setSpecular(col);
      break;
    }
  }
  if (currMat)
    materials.push_back(currMat);

  fclose(f);
  return true;
}
