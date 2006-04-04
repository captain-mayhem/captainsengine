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

#include "common.hh"
#include "texture.hh"
#include "world.hh"
#include "field.hh"
#include "matrix.hh"
#include "door.hh"

extern string path;

#ifdef WIN32
#define M_PI 3.1415926535897932384626433832795
#endif

//CONSTRUCTOR
Door::Door(){
  script_ = NULL;
}

//Copy Constructor
Door::Door(const Door& d){
  position_ = d.position_;
  id_ = d.id_;
  active_ = d.active_;
  interpolate_ = d.interpolate_;

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
  if (!active_)
    return;
  
  glColor3f(1,1,1);
  glEnable(GL_TEXTURE_2D);
  short id = 0;
  //normal door
  if (id_ == 0)
    id = 1;
  glBindTexture(GL_TEXTURE_2D, tex.wallTex[id]);
  glPushMatrix();
  
  //a door is just a textured, properly scaled cube
  //horizontal doors
  if (position_.x == pos2_.x){
    Matrix t(Matrix::Translation, Vector3D(position_.x*QUADSIZE+QUADSIZE/2,1.0/3*WALLHEIGHT,position_.y*QUADSIZE+QUADSIZE));
    Matrix s(Matrix::Scale, Vector3D((QUADSIZE-2*THICKNESS)/2,1.0/3*WALLHEIGHT,THICKNESS));
    //secret door?
    if (id_ > 0){
	    /*
      glTranslatef(position_.x*QUADSIZE+QUADSIZE/2,1.0/3*WALLHEIGHT,position_.y*QUADSIZE+QUADSIZE);
      glScalef((QUADSIZE-2*THICKNESS)/2,1.0/3*WALLHEIGHT,THICKNESS);
      */
    }
    else{
	    s = s*Matrix(Matrix::Scale, Vector3D(1,1,0.5));
	    /*
      glTranslatef(position_.x*QUADSIZE+QUADSIZE/2,1.0/3*WALLHEIGHT,position_.y*QUADSIZE+QUADSIZE);
      glScalef((QUADSIZE-2*THICKNESS)/2,1.0/3*WALLHEIGHT,THICKNESS*0.5);
      */
    }
    (t*s).toOpenGL(); 
  }
  //vertical doors
  else{
  Matrix r(Matrix::Rotation, Vector3D(0,1,0), M_PI/2);
    Matrix t(Matrix::Translation, Vector3D(position_.x*QUADSIZE+QUADSIZE,1.0/3*WALLHEIGHT,position_.y*QUADSIZE+QUADSIZE/2));
    Matrix s(Matrix::Scale, Vector3D((QUADSIZE-2*THICKNESS)/2,1.0/3*WALLHEIGHT,THICKNESS));
    if (id_ > 0){
	    /*
      glTranslatef(position_.x*QUADSIZE+QUADSIZE,1.0/3*WALLHEIGHT,position_.y*QUADSIZE+QUADSIZE/2);
      glScalef(THICKNESS,1.0/3*WALLHEIGHT,(QUADSIZE-2*THICKNESS)/2);
      */
    }
    else{
	    s = s*Matrix(Matrix::Scale, Vector3D(1,1,0.5));
	    /*
      glTranslatef(position_.x*QUADSIZE+QUADSIZE,1.0/3*WALLHEIGHT,position_.y*QUADSIZE+QUADSIZE/2);
      glScalef(THICKNESS*0.5,1.0/3*WALLHEIGHT,(QUADSIZE-2*THICKNESS)/2);
      */
    }
    (t*r*s).toOpenGL();
  }
 
  //secret door
  if (id_ > 0){
    float xr = (float)THICKNESS/QUADSIZE;
    float xl = (float)(QUADSIZE-THICKNESS)/QUADSIZE;
    float yu = 0;
    float yd = 2*2.0/3;
    float textureCoords[] = {
      xl,yu,
      xr,yu,
      xl,yu,
      xr,yu,
      xl,yd,
      xr,yd,
      xl,yd,
      xr,yd
    };
    Forms::drawCube(textureCoords);
  }
  else{
    Forms::drawCube();
  }
  glPopMatrix();

#endif
  
}

//2D rendering of doors takes place in world.cpp
void Door::render2D() const{
}

//build collision vertices
void Door::update(){
  //This code is pretty ugly to read. Drawing the situation on a sheet of paper will help.
  
  //horizontal door
  if (position_.x == pos2_.x){
    verts_[0] = Vector3D(position_.x*QUADSIZE+THICKNESS, DOORHEIGHT, position_.y*QUADSIZE+QUADSIZE-0.5*THICKNESS);
    verts_[1] = Vector3D(position_.x*QUADSIZE+QUADSIZE-THICKNESS, DOORHEIGHT, position_.y*QUADSIZE+QUADSIZE-0.5*THICKNESS);
    verts_[2] = Vector3D(position_.x*QUADSIZE+THICKNESS, 0, position_.y*QUADSIZE+QUADSIZE-0.5*THICKNESS);
    verts_[3] = Vector3D(position_.x*QUADSIZE+QUADSIZE-THICKNESS, 0, position_.y*QUADSIZE+QUADSIZE-0.5*THICKNESS);
    
    verts_[4] = Vector3D(position_.x*QUADSIZE+QUADSIZE-THICKNESS, DOORHEIGHT, position_.y*QUADSIZE+QUADSIZE+0.5*THICKNESS);
    verts_[5] = Vector3D(position_.x*QUADSIZE+THICKNESS, DOORHEIGHT, position_.y*QUADSIZE+QUADSIZE+0.5*THICKNESS);
    verts_[6] = Vector3D(position_.x*QUADSIZE+QUADSIZE-THICKNESS, 0, position_.y*QUADSIZE+QUADSIZE+0.5*THICKNESS);
    verts_[7] = Vector3D(position_.x*QUADSIZE+THICKNESS, 0, position_.y*QUADSIZE+QUADSIZE+0.5*THICKNESS);
  }
  //vertical door
  else{
    verts_[0] = Vector3D(position_.x*QUADSIZE+QUADSIZE-0.5*THICKNESS, DOORHEIGHT, position_.y*QUADSIZE+QUADSIZE-THICKNESS);
    verts_[1] = Vector3D(position_.x*QUADSIZE+QUADSIZE-0.5*THICKNESS, DOORHEIGHT, position_.y*QUADSIZE+THICKNESS);
    verts_[2] = Vector3D(position_.x*QUADSIZE+QUADSIZE-0.5*THICKNESS, 0, position_.y*QUADSIZE+QUADSIZE-THICKNESS);
    verts_[3] = Vector3D(position_.x*QUADSIZE+QUADSIZE-0.5*THICKNESS, 0, position_.y*QUADSIZE+THICKNESS);
    
    verts_[4] = Vector3D(position_.x*QUADSIZE+QUADSIZE+0.5*THICKNESS, DOORHEIGHT, position_.y*QUADSIZE+THICKNESS);
    verts_[5] = Vector3D(position_.x*QUADSIZE+QUADSIZE+0.5*THICKNESS, DOORHEIGHT, position_.y*QUADSIZE+QUADSIZE-THICKNESS);
    verts_[6] = Vector3D(position_.x*QUADSIZE+QUADSIZE+0.5*THICKNESS, 0, position_.y*QUADSIZE+THICKNESS);
    verts_[7] = Vector3D(position_.x*QUADSIZE+QUADSIZE+0.5*THICKNESS, 0, position_.y*QUADSIZE+QUADSIZE-THICKNESS);
  }
}

//write a door to a stream
void Door::write(ofstream& out){
  out.write((char*)&id_, sizeof(id_));
  out.write((char*)&active_, sizeof(active_));
  out.write((char*)&position_, sizeof(position_));
  out.write((char*)&pos2_, sizeof(pos2_));
}

//read a door from a stream
void Door::read(ifstream& in){
  in.read((char*)&id_, sizeof(id_));
  in.read((char*)&active_, sizeof(active_));
  in.read((char*)&position_, sizeof(position_));
  in.read((char*)&pos2_, sizeof(pos2_));
}

