#include "material.h"
#include <system/file.h>
#include <system/engine.h>

using namespace CGE;

Material::Material(std::string const& name) : mName(name){
  mAmbient = Color(0.0f,0.0f,0.0f,1.0f);
  mDiffuse = Color(1.0f,1.0f,1.0f,1.0f);
  mSpecular = Color(0.0f,0.0f,0.0f,1.0f);
  mSpecPower = 10.0f;
  mOpacity = 1.0f;
}

Material::~Material(){
}

bool Material::loadFromMTL(std::string const& filename, std::vector<Material*>& materials){
  FILE* f = fopen(filename.c_str(), "r");
  if (!f)
    return false;

  char line[2000];
  char* pos;
  
  Material* currMat = NULL;
  Color col;

  while (fgets(line, 2000, f)){
    switch (line[0]){
    case 'd':
      {
        float opacity = strtof(line + 2, NULL);
        currMat->setOpacity(opacity);
      }
      break;
    case 'm':
      if (memcmp(line, "map_Kd", 6) == 0){
        std::string texture(line + 7);
        texture.resize(texture.size()-1);
        std::string path = Filesystem::getPathComponent(filename);
        texture = Filesystem::combinePath(path, texture);
        currMat->setDiffuseTexFile(texture);
      }
      else if (memcmp(line, "map_d", 5) == 0){
        int count = 6;
        while (isspace(line[count]))
          ++count;
        std::string texture(line + count);
        texture.resize(texture.size()-1);
        std::string path = Filesystem::getPathComponent(filename);
        texture = Filesystem::combinePath(path, texture);
        currMat->setOpacityTexFile(texture);
      }
      break;
    case 'n':
      if (memcmp(line, "newmtl", 6) == 0){
        std::string mtl(line + 7);
        mtl.resize(mtl.size()-1);
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
