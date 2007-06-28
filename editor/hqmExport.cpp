
#include "hqmExport.h"

#include "mesh/model.h"
#include "renderer/scene.h"
#include "math/matrix.h"
#include "math/vector.h"
#include "system/engine.h"

#include <math.h>
#include <list>

using std::list;
using MeshGeo::Model;
using Math::Matrix;
using Math::Vector3D;
using System::Log;

HQMExport::HQMExport(){

}

HQMExport::~HQMExport(){

}

bool HQMExport::exportHQM(Graphics::Scene& scn, const std::string& filename){
  const list<Model*>& models = scn.getModels();
  //determine size of the map
  list<Model*>::const_iterator iter;
  int sizeX = 0;
  int sizeY = 0;
  for (iter = models.begin(); iter != models.end(); iter++){
    Matrix mat = (*iter)->getTrafo();
    Vector3D trans = mat.getTranslation();
    int tmpX = (int)((trans.x-4.0f)/8.0f+1.0f);
    int tmpY = (int)((trans.z-4.0f)/8.0f+1.0f);
    sizeX = max(sizeX, tmpX);
    sizeY = max(sizeY, tmpY);
  }
  Log << sizeX << " " << sizeY;
  return false;
}