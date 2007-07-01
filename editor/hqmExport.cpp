
#include "hqmExport.h"
#include "editor.h"

#include "mesh/model.h"
#include "renderer/scene.h"
#include "math/matrix.h"
#include "system/engine.h"

#include <math.h>
#include <list>

using std::list;
using std::max;
using MeshGeo::Model;
using Math::Matrix;
using Math::Vector3D;
using Math::Vector2D;
using System::Log;

HQMExport::HQMExport(){
  width_ = 0;
  height_ = 0;
  map_ = NULL;
}

HQMExport::~HQMExport(){
  //deallocate map
  for (int i = 0; i < height_; i++){
    delete [] map_[i];
  }
  delete [] map_;
  map_ = NULL;
}

bool HQMExport::exportHQM(Graphics::Scene& scn, const std::string& filename){
  const list<Model*>& models = scn.getModels();
  //determine size of the map
  list<Model*>::const_iterator iter;
  for (iter = models.begin(); iter != models.end(); iter++){
    Matrix mat = (*iter)->getTrafo();
    Vector3D trans = mat.getTranslation();
    Vector2D pos = convertToMap(trans);
    short tmpX = pos.x;
    short tmpY = pos.y;
    width_ = max(width_, tmpX);
    height_ = max(height_, tmpY);
  }
  width_++; height_++;
  //allocate the map
  map_ = new Field*[height_];
  for (int i = 0; i < height_; i++){
    map_[i] = new Field[width_];
  }
  //extract the informations needed for the map
  for (iter = models.begin(); iter != models.end(); iter++){
    Matrix mat = (*iter)->getTrafo();
    Vector3D trans = mat.getTranslation();
    Vector2D pos = convertToMap(trans);
    Field& f = map_[pos.y][pos.x];
    f.position = pos;
    int classAttrib = (*iter)->getAttrib(0);
    //ground
    if (classAttrib == Editor::GROUND){
      f.id = (*iter)->getAttrib(1);
    }
    //wall
    if (classAttrib == Editor::WALL){
      f.wallbits.set(extractDir(mat));
    }
  }
  //print out map
  for(int j = 0; j < height_; j++){
    for (int i = 0; i < width_; i++){
      int tmp = map_[j][i].id;
      if (tmp < 10)
        Log << 0;
      Log << tmp<<" ";
    }
    Log << std::endl;
  }
  return false;
}

//! Get the field position given a world position
Vector2D HQMExport::convertToMap(const Vector3D& pos){
  Vector2D result;
  result.x = (int)((pos.x-4.0f)/8.0f);
  result.y = (int)((pos.z-4.0f)/8.0f);
  return result;
}

//! Get the direction given a matrix
HQMExport::Direction HQMExport::extractDir(const Math::Matrix& mat){
  //it is assumed that the matrix contains a rotation around the axis (0,1,0).
  Vector3D orig = Vector3D(1,0,1).normalized();
  Vector3D rotated = mat*orig;
  rotated.normalize();
  float angle = acos(orig.dot(rotated));
  angle = angle / M_PI*180;
  //correct quadrant
  angle = rotated.x < 0 ? 360-angle : angle;
  if (angle >= 315 || angle < 45)
    return TOP;
  else if (angle >= 45 && angle < 135)
    return RIGHT;
  else if (angle >= 135 && angle < 225)
    return BOTTOM;
  else
    return LEFT;
}

