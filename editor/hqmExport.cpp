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
#include <sstream>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using std::list;
using std::max;
using std::ofstream;
using std::ios;
using CGE::Model;
using CGE::Matrix;
using CGE::Vector3D;
using CGE::Vector2D;

TR_CHANNEL(HQ_Editor_HQM);

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

bool HQMExport::exportHQM(CGE::Scene& scn, const std::string& filename){
  TR_USE(HQ_Editor_HQM);
  const list<Model*>& models = scn.getModels();
  //determine size of the map
  list<Model*>::const_iterator iter;
  for (iter = models.begin(); iter != models.end(); iter++){
    Matrix mat = (*iter)->getTrafo();
    Vector3D trans = mat.getTranslation();
    Vector2D pos = convertToMap(trans, 0.5f, 0.5f);
    short tmpX = pos.x;
    short tmpY = pos.y;
    width_ = max(width_, tmpX);
    height_ = max(height_, tmpY);
  }
  width_++; height_++;
  //allocate the map
  map_ = new HQField*[height_];
  for (int i = 0; i < height_; i++){
    map_[i] = new HQField[width_];
  }
  //extract the informations needed for the map
  for (iter = models.begin(); iter != models.end(); iter++){
    Matrix mat = (*iter)->getTrafo();
    Vector3D trans = mat.getTranslation();
    Vector2D pos = convertToMap(trans, 0.5f, 0.5f);
    HQField* f = &map_[pos.y][pos.x];
    int classAttrib = (*iter)->getAttrib(0);
    //ground
    if (classAttrib == Editor::GROUND){
      f->position = pos;
      f->id = (*iter)->getAttrib(1);
      //update rooms
      while (rooms_.size() <= (unsigned short)f->id){
        vector<Vector2D> tmp;
        rooms_.push_back(tmp);
      }
      vector<Vector2D>* room = &rooms_[f->id];
      room->push_back(pos);
      f->modelids.push_back((*iter)->getID());
      //std::cerr << (*iter)->getID();
    }
    //wall
    else if (classAttrib == Editor::WALL){
      Direction d = extractDir(mat);
      //std::cerr << d << " " << pos << "\n";
      f->wallbits.set(d);
      f->modelids.push_back((*iter)->getID());
    }
    //wallpart
    else if (classAttrib == Editor::WALLPART){
      f->modelids.push_back((*iter)->getID());
    }
    //starting position
    else if (classAttrib == Editor::STARTPOS){
      startpos_.push_back(pos);
    }
    //door
    else if (classAttrib == Editor::DOOR){
      HQDoor door;
      door.id = (*iter)->getID();
      door.type = (*iter)->getAttrib(1);
      int idx = (int)doors_.size();
      Direction d = extractDir(mat);
      f->doorbits.set(d);
      f->dooridx[d] = idx;
      if (d == TOP){
        door.pos = Vector2D(pos.x,pos.y-1);
        door.pos2 = pos;
        HQField* f2 = &map_[pos.y-1][pos.x];
        f2->doorbits.set(BOTTOM);
        f2->dooridx[BOTTOM] = idx;
      }
      else if (d == RIGHT){
        door.pos = pos;
        door.pos2 = Vector2D(pos.x+1,pos.y);
        HQField* f2 = &map_[pos.y][pos.x+1];
        f2->doorbits.set(LEFT);
        f2->dooridx[LEFT] = idx;
      }
      else if (d == BOTTOM){
        door.pos = pos;
        door.pos2 = Vector2D(pos.x,pos.y+1);
        HQField* f2 = &map_[pos.y+1][pos.x];
        f2->doorbits.set(TOP);
        f2->dooridx[TOP] = idx;
      }
      else if (d == LEFT){
        door.pos = Vector2D(pos.x-1,pos.y);
        door.pos2 = pos;
        HQField* f2 = &map_[pos.y][pos.x-1];
        f2->doorbits.set(RIGHT);
        f2->dooridx[RIGHT] = idx;
      }
      doors_.push_back(door);
    }

    //monsters
    else if (classAttrib == Editor::MONSTER){
      Monster m;
      m.id = (*iter)->getID();
      m.instanceid = (*iter)->getAttrib(1);
      m.pos = pos;
      monsters_.push_back(m);
    }
    else if (classAttrib == Editor::FURNITURE){
      Furniture f;
      f.id = (*iter)->getID();
      f.instanceid = (*iter)->getAttrib(1);
      int w = (*iter)->getAttrib(2);
      int h = (*iter)->getAttrib(3);
      Direction d = extractDir(mat);
      f.direction = (unsigned)d;

      float xoffset = w*0.5f;
      float yoffset = h*0.5f;
      if (d == LEFT){
        xoffset = h*0.5f;
        yoffset = 1.0f - w*0.5f;
      }
      else if (d == BOTTOM){
        xoffset = 1.0f - w*0.5f;
        yoffset = 1.0f - h*0.5f;
      }
      if (d == RIGHT){
        xoffset = 1.0f - h*0.5f;
        yoffset = w*0.5f;
      }

      f.pos = convertToMap(trans, xoffset, yoffset);
      furnitures_.push_back(f);
    }
    else if (classAttrib == Editor::OVERLAY){
      HQOverlay o;
      o.id = (*iter)->getID();
      o.instanceid = (*iter)->getAttrib(1);
      int w = (*iter)->getAttrib(2);
      int h = (*iter)->getAttrib(3);
      Direction d = extractDir(mat);
      o.direction = (unsigned)d;

      float xoffset = w*0.5f;
      float yoffset = h*0.5f;
      if (d == LEFT){
        xoffset = h*0.5f;
        yoffset = 1.0f - w*0.5f;
      }
      else if (d == BOTTOM){
        xoffset = 1.0f - w*0.5f;
        yoffset = 1.0f - h*0.5f;
      }
      if (d == RIGHT){
        xoffset = 1.0f - h*0.5f;
        yoffset = w*0.5f;
      }

      o.pos = convertToMap(trans, xoffset, yoffset);
      overlays_.push_back(o);
    }
    else if (classAttrib == Editor::SCRIPT){
      HQScript s;
      s.id = (*iter)->getID();
      s.instanceid = (*iter)->getAttrib(2);
      s.pos = pos;
      scripts_.push_back(s);
    }
  }
  
  //write
  ofstream out(filename.c_str(), ios::binary);
  float version = 0.7f;
  out.write((char*)&version, sizeof(version));
  out.write((char*)&width_, sizeof(width_));
  out.write((char*)&height_, sizeof(height_));
  for (int j = 0; j < height_; j++){
    for(int i = 0; i < width_; i++){
      //write field content
      HQField* f = &map_[j][i];
      char bits = f->wallbits.getData();
      out.write(&bits, sizeof(bits));
      out.write((char*)&f->position, sizeof(f->position));
      out.write((char*)&f->id, sizeof(f->id));
      out.write((char*)&f->active, sizeof(f->active));
      bits = f->doorbits.getData();
      out.write(&bits, sizeof(bits));
      out.write((char*)f->dooridx, 4*sizeof(f->dooridx[0]));
      unsigned size = f->modelids.size();
      out.write((char*)&size, sizeof(size));
      for (unsigned i = 0; i < size; ++i){
        out.write((char*)&f->modelids[i], sizeof(unsigned));
      }
    }
  }
  //hero starting positions
  unsigned short size = startpos_.size();
  out.write((char*)&size, sizeof(size));
  for (unsigned i = 0; i < size; ++i){
    out.write((char*)&startpos_[i], sizeof(Vector2D));
  }
  
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

  //doors
  size = doors_.size();
  out.write((char*)&size, sizeof(size));
  for (unsigned i = 0; i < size; ++i){
    HQDoor door = doors_[i];
    out.write((char*)&door.id, sizeof(door.id));
    out.write((char*)&door.type, sizeof(door.type));
    out.write((char*)&door.pos, sizeof(door.pos));
    out.write((char*)&door.pos2, sizeof(door.pos2));
  }
  
  //monsters
  size = monsters_.size();
  out.write((char*)&size, sizeof(size));
  for (unsigned i = 0; i < size; ++i){
    Monster m = monsters_[i];
    out.write((char*)&m.id, sizeof(m.id));
    out.write((char*)&m.instanceid, sizeof(m.instanceid));
    out.write((char*)&m.pos, sizeof(m.pos));
    //out.write((char*)&m, sizeof(m));
  }
  //furniture
  size = furnitures_.size();
  out.write((char*)&size, sizeof(size));
  for (unsigned i = 0; i < size; ++i){
    Furniture& f = furnitures_[i];
    out.write((char*)&f.id, sizeof(f.id));
    out.write((char*)&f.instanceid, sizeof(f.instanceid));
    out.write((char*)&f.pos, sizeof(f.pos));
    out.write((char*)&f.direction, sizeof(f.direction));
  }
  //overlays
  size = overlays_.size();
  out.write((char*)&size, sizeof(size));
  for (unsigned i = 0; i < size; ++i){
    HQOverlay& o = overlays_[i];
    out.write((char*)&o.id, sizeof(o.id));
    out.write((char*)&o.instanceid, sizeof(o.instanceid));
    out.write((char*)&o.pos, sizeof(o.pos));
    out.write((char*)&o.direction, sizeof(o.direction));
  }
  //scripts
  size = scripts_.size();
  out.write((char*)&size, sizeof(size));
  for (unsigned i = 0; i < size; ++i){
    HQScript& s = scripts_[i];
    out.write((char*)&s.id, sizeof(s.id));
    out.write((char*)&s.instanceid, sizeof(s.instanceid));
    out.write((char*)&s.pos, sizeof(s.pos));
  }

  out.close();
  
  //print out map
  for(int j = 0; j < height_; j++){
    std::ostringstream Log;
    for (int i = 0; i < width_; i++){
      int tmp = map_[j][i].id;
      if (tmp < 10)
        Log << 0;
      Log << tmp<<" ";
    }
    TR_INFO(Log.str().c_str());
  }
  return false;
}

//! Get the field position given a world position
Vector2D HQMExport::convertToMap(const Vector3D& pos, float xoffset, float yoffset){
  Vector2D result;
  result.x = (int)((pos.x-8.0f*xoffset)/8.0f);
  result.y = (int)((pos.z-8.0f*yoffset)/8.0f);
  return result;
}

//! Get the direction given a matrix
HQMExport::Direction HQMExport::extractDir(const CGE::Matrix& mat){
  //it is assumed that the matrix contains a rotation around the axis (0,1,0).
  Vector3D orig = Vector3D(0,0,1);
  Vector3D rotated1 = mat*orig;
  Vector3D rotated2 = mat*(orig*-1);
  Vector3D rotated = (rotated1 - rotated2).normalized();
  float angle = acos(orig.dot(rotated));
  angle = angle / (float)M_PI*180;
  //std::cerr << "*" << rotated << "!" << angle << "*";
  //correct quadrant
  angle = rotated.x < 0 ? 360-angle : angle;
  if (angle >= 315 || angle < 45)
    return TOP;
  else if (angle >= 45 && angle < 135)
    return LEFT;
  else if (angle >= 135 && angle < 225)
    return BOTTOM;
  else
    return RIGHT;
}

