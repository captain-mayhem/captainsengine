//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: door.cpp                                                   |
//  ==================================================================

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <string>

#include "system/engine.h"
#include "renderer/renderer.h"

#include "common.h"
//#include "../HQEditor/compiler.h"
#include "textureManager.h"
#include "world.h"
#include "field.h"
#include "math/matrix.h"
#include "door.h"

using std::string;

using CGE::Matrix;

extern string path;

#ifdef WIN32
#define M_PI 3.1415926535897932384626433832795
#endif

//CONSTRUCTOR
Door::Door(){
  script_ = NULL;
  closed_ = true;
  model_ = NULL;
}

//Copy Constructor
Door::Door(const Door& d){
  pos_ = d.pos_;
  type_ = d.type_;
  closed_ = d.closed_;

  pos2_ = d.pos2_;
  memcpy(verts_, d.verts_, 8*sizeof(verts_[0]));  
  script_ = d.script_;
}

//DESTRUCTOR
Door::~Door(){
}

//Model loading, when in the future doors will no longer be 
//simple cubes but models from a modelling tool
void Door::load(){
}

//Render the door
void Door::render() const{
//the door class is shared by server and client, but only the client needs rendering
#if defined _CLIENT_
  
  //don't draw open doors.
  if (!closed_)
    return;

  model_->setupMaterial();
  model_->render();
  model_->resetMaterial();
#endif
  
}

//2D rendering of doors takes place in world.cpp
void Door::render2D() const{
}

//build collision vertices
void Door::update(){
  //This code is pretty ugly to read. Drawing the situation on a sheet of paper will help.
  
  //horizontal door
  if (pos_.x == pos2_.x){
    verts_[0] = Vector3D(pos_.x*QUADSIZE+THICKNESS, DOORHEIGHT, pos_.y*QUADSIZE+QUADSIZE-0.5f*THICKNESS);
    verts_[1] = Vector3D(pos_.x*QUADSIZE+QUADSIZE-THICKNESS, DOORHEIGHT, pos_.y*QUADSIZE+QUADSIZE-0.5f*THICKNESS);
    verts_[2] = Vector3D(pos_.x*QUADSIZE+THICKNESS, 0, pos_.y*QUADSIZE+QUADSIZE-0.5f*THICKNESS);
    verts_[3] = Vector3D(pos_.x*QUADSIZE+QUADSIZE-THICKNESS, 0, pos_.y*QUADSIZE+QUADSIZE-0.5f*THICKNESS);
    
    verts_[4] = Vector3D(pos_.x*QUADSIZE+QUADSIZE-THICKNESS, DOORHEIGHT, pos_.y*QUADSIZE+QUADSIZE+0.5f*THICKNESS);
    verts_[5] = Vector3D(pos_.x*QUADSIZE+THICKNESS, DOORHEIGHT, pos_.y*QUADSIZE+QUADSIZE+0.5f*THICKNESS);
    verts_[6] = Vector3D(pos_.x*QUADSIZE+QUADSIZE-THICKNESS, 0, pos_.y*QUADSIZE+QUADSIZE+0.5f*THICKNESS);
    verts_[7] = Vector3D(pos_.x*QUADSIZE+THICKNESS, 0, pos_.y*QUADSIZE+QUADSIZE+0.5f*THICKNESS);
  }
  //vertical door
  else{
    verts_[0] = Vector3D(pos_.x*QUADSIZE+QUADSIZE-0.5f*THICKNESS, DOORHEIGHT, pos_.y*QUADSIZE+QUADSIZE-THICKNESS);
    verts_[1] = Vector3D(pos_.x*QUADSIZE+QUADSIZE-0.5f*THICKNESS, DOORHEIGHT, pos_.y*QUADSIZE+THICKNESS);
    verts_[2] = Vector3D(pos_.x*QUADSIZE+QUADSIZE-0.5f*THICKNESS, 0, pos_.y*QUADSIZE+QUADSIZE-THICKNESS);
    verts_[3] = Vector3D(pos_.x*QUADSIZE+QUADSIZE-0.5f*THICKNESS, 0, pos_.y*QUADSIZE+THICKNESS);
    
    verts_[4] = Vector3D(pos_.x*QUADSIZE+QUADSIZE+0.5f*THICKNESS, DOORHEIGHT, pos_.y*QUADSIZE+THICKNESS);
    verts_[5] = Vector3D(pos_.x*QUADSIZE+QUADSIZE+0.5f*THICKNESS, DOORHEIGHT, pos_.y*QUADSIZE+QUADSIZE-THICKNESS);
    verts_[6] = Vector3D(pos_.x*QUADSIZE+QUADSIZE+0.5f*THICKNESS, 0, pos_.y*QUADSIZE+THICKNESS);
    verts_[7] = Vector3D(pos_.x*QUADSIZE+QUADSIZE+0.5f*THICKNESS, 0, pos_.y*QUADSIZE+QUADSIZE-THICKNESS);
  }
}

//write a door to a stream
void Door::write(ofstream& out){
  out.write((char*)&type_, sizeof(type_));
  //out.write((char*)&active_, sizeof(active_));
  out.write((char*)&pos_, sizeof(pos_));
  out.write((char*)&pos2_, sizeof(pos2_));
}

//read a door from a stream
void Door::read(ifstream& in, CGE::Scene& scene){
  unsigned modelid;
  in.read((char*)&modelid, sizeof(modelid));
#ifdef _CLIENT_
  model_ = scene.getModel(modelid);
#endif
  in.read((char*)&type_, sizeof(type_));
  //in.read((char*)&active_, sizeof(active_));
  in.read((char*)&pos_, sizeof(pos_));
  in.read((char*)&pos2_, sizeof(pos2_));
}

