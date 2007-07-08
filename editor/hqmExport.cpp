#define _USE_MATH_DEFINES
#include <math.h>

#include "hqmExport.h"
#include "editor.h"

#include "mesh/model.h"
#include "renderer/scene.h"
#include "math/matrix.h"
#include "system/engine.h"

#include <math.h>
#include <list>
#include <fstream>
#include <iostream>

using std::list;
using std::max;
using std::ofstream;
using std::ios;
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
    int classAttrib = (*iter)->getAttrib(0);
    //ground
    if (classAttrib == Editor::GROUND){
      f.position = pos;
      f.id = (*iter)->getAttrib(1);
      //update rooms
      while (rooms_.size() <= (unsigned short)f.id){
        vector<Vector2D> tmp;
        rooms_.push_back(tmp);
      }
      vector<Vector2D>* room = &rooms_[f.id];
      room->push_back(pos);
    }
    //wall
    if (classAttrib == Editor::WALL){
      f.wallbits.set(extractDir(mat));
    }
    //door
    {
    }
  }
  
  //write
  ofstream out(filename.c_str(), ios::binary);
  float version = 0.7;
  out.write((char*)&version, sizeof(version));
  out.write((char*)&width_, sizeof(width_));
  out.write((char*)&height_, sizeof(height_));
  for (int j = 0; j < height_; j++){
    for(int i = 0; i < width_; i++){
      //write field content
      Field& f = map_[j][i];
      char bits = f.wallbits.getData();
      out.write(&bits, sizeof(bits));
      out.write((char*)&f.position, sizeof(f.position));
      out.write((char*)&f.id, sizeof(f.id));
      out.write((char*)&f.active, sizeof(f.active));
      bits = f.doorbits.getData();
      out.write(&bits, sizeof(bits));
      out.write((char*)f.dooridx, 4*sizeof(f.dooridx[0]));
    }
  }
  //hero starting positions
  unsigned short size;
  //TODO
  
  //room structure
  size = rooms_.size();
  out.write((char*)&size, sizeof(size));
  for (unsigned i = 0; i < size; i++){
    vector<Vector2D>& temp = rooms_[i];
    unsigned short fields = temp.size();
    out.write((char*)&fields, sizeof(fields));
    for (unsigned j = 0; j < fields; j++){
      Vector2D v = temp[j];
      out.write((char*)&v, sizeof(v));
    }
  }

  //TODO more data
  
  out.close();
  
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
  angle = angle / (float)M_PI*180;
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

