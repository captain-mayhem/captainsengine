#include "material.h"

using namespace CGE;

Material::Material(std::string const& name) : mName(name){
  mAmbient = Color(0.0f,0.0f,0.0f,1.0f);
  mDiffuse = Color(1.0f,1.0f,1.0f,1.0f);
  mSpecular = Color(0.0f,0.0f,0.0f,1.0f);
  mPower = 0.0f;
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
