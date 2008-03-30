//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// | Graphics Programming and Applications (Assignment 5)             |
// |                                                                  |
// | Marco Staginski                                                  |
// | staginski@gmx.de                                                 |
// |------------------------------------------------------------------|
// | File: compiler.cpp                                               |
//  ==================================================================

#define _USE_MATH_DEFINES
#include <iostream>
#include <fstream>
#include <cassert>

#include "system/engine.h"
#include "mesh/mesh.h"
#include "renderer/texture.h"
#include "renderer/renderer.h"

#include "../HQClient/common.h"
#include "compiler.h"

using std::ofstream;
using std::ios;
using std::cerr;
using std::cout;
using std::endl;

extern string path;

using namespace Math;
using System::Log;


//Constructor, takes map size
Compiler::Compiler(short width, short height){
  width_ = width;
  height_ = height;
}

//Destructor
Compiler::~Compiler(){
}

//Set a wall
void Compiler::setWall(const Vector2D& pos, Direction d){
  std::string tilename = "wallstd.obj";
  Math::Matrix rot;

  short length = getLengthInfo(pos, d, &rot);
  if (length == 3)
    tilename = "wallRlLl.obj";
  else if (length == 2)
    tilename = "wallRl.obj";
  else if (length == 1)
    tilename = "wallLl.obj";
  
   //see if the wall tile is in there
  MeshGeo::Mesh* msh = getMesh(tilename, "models/world/");

  //build texture name
  std::string texName;
  texName = "textures/world/floorWall.jpg";
  
  //get texture
  Graphics::Texture* tex = getTexture(texName);
  
  MeshGeo::Model* mdl = new MeshGeo::Model(msh);
  mdl->assignTexture(tex, 0);
  //wall
  mdl->setAttrib(0, 1001);
  //mdl->setAttrib(1, id);
  Math::Matrix mat(Matrix::Translation, Vector3D((float)(pos.x*8+4),0,(float)(pos.y*8+4)));
  mdl->setTrafo(mat*rot);
  scene_.addModel(mdl);
}

//is it a wall?
bool Compiler::isWall(short x, short y, Direction dir) const{
  if (isDoor(x,y,dir))
    return true;
  if (x < 0 || x >= width_ || y < 0 || y >= height_)
    return false;
  char c;
  if (dir == TOP)
    c = map_[y*3][x*5+2];
  else if (dir == RIGHT)
    c = map_[y*3+1][x*5+5];
  else if (dir == BOTTOM)
    c = map_[y*3+3][x*5+2];
  else if (dir == LEFT)
    c = map_[y*3+1][x*5];
  if (c == '|' || c == '-')
    return true;
  return false;
}

//is it a door?
bool Compiler::isDoor(short x, short y, Direction dir) const{
  if (x < 0 || x >= width_ || y < 0 || y >= height_)
    return false;
  char c;
  if (dir == TOP)
    c = map_[y*3][x*5+2];
  else if (dir == RIGHT)
    c = map_[y*3+1][x*5+5];
  else if (dir == BOTTOM)
    c = map_[y*3+3][x*5+2];
  else if (dir == LEFT)
    c = map_[y*3+1][x*5];
  if (c == 'H' || c == 'D' || c == 'S')
    return true;
  return false;
}
	
//set a door
void Compiler::setDoor(const Vector2D& pos, Direction d, short type){
  //add the wall part of the door
  std::string tilename = "doorstd.obj";
  Math::Matrix rot;

  short length = getLengthInfo(pos, d, &rot);
  if (length == 3)
    tilename = "doorRlLl.obj";
  else if (length == 2)
    tilename = "doorRl.obj";
  else if (length == 1)
    tilename = "doorLl.obj";

  //see if the wall tile is in there
  MeshGeo::Mesh* msh = getMesh(tilename, "models/world/");

  //build texture name
  std::string texName;
  texName = "textures/world/floorWall.jpg";

  //get texture
  Graphics::Texture* tex = getTexture(texName);

  MeshGeo::Model* mdl = new MeshGeo::Model(msh);
  mdl->assignTexture(tex, 0);
  //wall
  mdl->setAttrib(0, 1004);
  Math::Matrix mat(Matrix::Translation, Vector3D((float)(pos.x*8+4),0,(float)(pos.y*8+4)));
  mdl->setTrafo(mat*rot);
  scene_.addModel(mdl);

  
  //now add the real door
  //we want only one door (parsed two times)
  //TODO level entry/exit doors
  if (d == BOTTOM || d == LEFT)
    return;

  if (d == RIGHT)
    rot = Math::Matrix(Matrix::Rotation, Vector3D(0,1,0), (float)(270.0/180.0*M_PI));
  else
    rot = Math::Matrix(Matrix::Identity);

  //standard
  if (type == 0){
    tilename = "door.obj";
    texName = "textures/world/door.jpg";
  }
  //secret or hidden
  else{
    tilename = "door_hidden.obj";
    texName = "textures/world/floorWall.jpg";
  }
  
  msh = getMesh(tilename, "models/world/");
  mdl = new MeshGeo::Model(msh);

  tex = getTexture(texName);
  mdl->assignTexture(tex, 0);
  //door
  mdl->setAttrib(0, 1003);
  mdl->setAttrib(1, type);

  mat = Math::Matrix(Matrix::Translation, Vector3D((float)(pos.x*8+4),0,(float)(pos.y*8+4)));
  mdl->setTrafo(mat*rot);
  scene_.addModel(mdl);
}

//write the map as .hsf file
void Compiler::write(string filename){
  int idx = filename.find_last_of(".");
  filename.erase(idx);
  filename += ".hsf";
  scene_.save(filename);
}

//add a field to a room
void Compiler::addRoom(const Vector2D& pos, short id){
  //see if the ground tile is in there
  MeshGeo::Mesh* msh = getMesh("ground.obj", "models/world/");
  //build texture name
  std::string texName;
  if (id == 0)
    texName = "textures/world/floor.bmp";
  else
    texName = "textures/world/room"+toStr(id)+".bmp";

  //get texture
  Graphics::Texture* tex = getTexture(texName);
  
  MeshGeo::Model* mdl = new MeshGeo::Model(msh);
  mdl->assignTexture(tex, 0);
  //ground
  mdl->setAttrib(0, 1000);
  mdl->setAttrib(1, id);
  Math::Matrix mat(Matrix::Translation, Vector3D((float)(pos.x*8+4),0,(float)(pos.y*8+4)));
  mdl->setTrafo(mat);
  scene_.addModel(mdl);
}

//add hero starting position
void Compiler::addStartPos(const Vector2D& pos){
  //starts_.push_back(pos);

  MeshGeo::Mesh* msh = getMesh("startpos.obj", "models/world/");
  MeshGeo::Model* mdl = new MeshGeo::Model(msh);
  
  Graphics::Texture* tex = getTexture("textures/world/startpos1.jpg");
  mdl->assignTexture(tex, 0);
  //startpos
  mdl->setAttrib(0, 1002);
  Math::Matrix mat(Matrix::Translation, Vector3D((float)(pos.x*8+4),4,(float)(pos.y*8+4)));
  mdl->setTrafo(mat);
  scene_.addModel(mdl);
}

//add a monster
void Compiler::addMonster(const Vector2D& pos, char monster[2]){
  //resolve monster name
  std::string monstername;
  int monstervalue;
  short instanceid;
  short movement;
  short attack;
  short defend;
  short body;
  short mind;
  if (towlower(monster[0]) == 'g' && towlower(monster[1]) == 'o'){
    static int goblincount = 0;
    monstername = "goblin";
    monstervalue = 2000;
  }
  else if (towlower(monster[0]) == 's' && towlower(monster[1]) == 'k'){
    static int skeletoncount = 0;
    monstername = "skeleton";
    monstervalue = 2001;
  }
  else if (towlower(monster[0]) == 'z' && towlower(monster[1]) == 'o'){
    static int zombiecount = 0;
    monstername = "zombie";
    monstervalue = 2002;
  }
  else if (towlower(monster[0]) == 'o' && towlower(monster[1]) == 'r'){
    static int orccount = 0;
    monstername = "orc";
    monstervalue = 2003;
    instanceid = ++orccount;
    movement = 8;
    attack = 3;
    defend = 2;
    body = 1;
    mind = 2;
  }
  else if (towlower(monster[0]) == 'f' && towlower(monster[1]) == 'i'){
    monstername = "fimir";
    monstervalue = 2004;
  }
  else if (towlower(monster[0]) == 'm' && towlower(monster[1]) == 'u'){
    monstername = "mummy";
    monstervalue = 2005;
  }
  else if (towlower(monster[0]) == 'c' && towlower(monster[1]) == 'h'){
    monstername = "chaoswarrior";
    monstervalue = 2006;
  }
  else if (towlower(monster[0]) == 'g' && towlower(monster[1]) == 'a'){
    monstername = "gargoyle";
    monstervalue = 2007;
  }
  else if (towlower(monster[0]) == 'c' && towlower(monster[1]) == 'w'){
    monstername = "chaoswarlock";
    monstervalue = 2008;
  }
  else
    assert(false);
  //build monster model
  MeshGeo::Mesh* msh = getMesh(monstername+".obj", "models/monsters/");
  MeshGeo::Model* mdl = new MeshGeo::Model(msh);

  Graphics::Texture* tex = getTexture("textures/monsters/"+monstername+".jpg");
  mdl->assignTexture(tex, 0);
  //monster
  mdl->setAttrib(0, 1005);
  mdl->setAttrib(1, monstervalue);
  //mdl->setAttrib(2, instanceid);
  //mdl->setAttrib(3, movement);
  //mdl->setAttrib(4, attack);
  //mdl->setAttrib(5, defend);
  //mdl->setAttrib(6, body);
  //mdl->setAttrib(7, mind);
  Math::Matrix mat(Matrix::Translation, Vector3D((float)(pos.x*8+4),0,(float)(pos.y*8+4)));
  mdl->setTrafo(mat);
  scene_.addModel(mdl);
}

//add furniture
void Compiler::addFurniture(const Vector2D& pos, char furniture[2]){
  //resolve furniture name
  std::string furniturename;
  int furniturevalue;
  short width;
  short height;
  if (towlower(furniture[0]) == 'a' && towlower(furniture[1]) == 'b'){
    furniturename = "alchemists_bench";
    furniturevalue = 3000;
    width = 3;
    height = 2;
  }
  else if (towlower(furniture[0]) == 't' && towlower(furniture[1]) == 'a'){
    furniturename = "table";
    furniturevalue = 3007;
    width = 3;
    height = 2;
  }
  else
    return;
    //assert(false);
  MeshGeo::Mesh* msh = getMesh(furniturename+".obj", "models/furniture/");
  MeshGeo::Model* mdl = new MeshGeo::Model(msh);

  Graphics::Texture* tex = getTexture("textures/furniture/"+furniturename+".jpg");
  mdl->assignTexture(tex, 0);
  //furniture
  mdl->setAttrib(0, 1006);
  mdl->setAttrib(1, furniturevalue);
  //determine rotation
  float angle = 0;
  if (islower(furniture[0]) && islower(furniture[1]))
    angle = 0;
  else if (islower(furniture[0]) && !islower(furniture[1])){
    angle = M_PI/2.0;
    short tmp; tmp = width; width = height; height = tmp;
  }
  else if (!islower(furniture[0]) && islower(furniture[1]))
    angle = M_PI;
  else if (!islower(furniture[0]) && !islower(furniture[1])){
    angle = 3.0*M_PI/2.0;
    short tmp; tmp = width; width = height; height = tmp;
  }
  Math::Matrix rot(Matrix::Rotation, Vector3D(0,1,0), angle);
  Math::Matrix trans(Matrix::Translation, Vector3D((float)(pos.x*8+4),0,(float)(pos.y*8+4)));
  mdl->setTrafo(rot*trans);
  scene_.addModel(mdl);
}

//add overlay
void Compiler::addOverlay(const Vector2D& pos, char overlay[3]){
  overlayPos op;
  op.overlay[0] = overlay[0];
  op.overlay[1] = overlay[1];
  op.overlay[2] = '\0';
  op.pos = pos;
  overlays_.push_back(op);
}


void Compiler::addScript(char ident, const Vector2D& pos){
  scriptPos sp;
  sp.script[0] = ident;
  sp.script[1] = '\0';
  sp.pos = pos;
  scripts_.push_back(sp);
}

MeshGeo::Mesh* Compiler::getMesh(std::string tilename, std::string path){
  //see if the mesh is in there
  MeshGeo::Mesh* msh = NULL;
  std::vector<MeshGeo::Mesh*> meshes = scene_.getMeshes();
  for (unsigned  i = 0; i < meshes.size(); ++i){
    if (meshes[i]->getName() == path+tilename){
      msh = meshes[i];
      break;
    }
  }
  //load mesh
  if (!msh){
    msh = new MeshGeo::Mesh();
    msh->loadFromFile(path+tilename);
    scene_.addMesh(msh);
  }
  return msh;
}

Graphics::Texture* Compiler::getTexture(std::string texname){
  //see if the texture is in there
  Graphics::Texture* tex = NULL;
  std::vector<Graphics::Texture*> textures = scene_.getTextures();
  for (unsigned i = 0; i < textures.size(); ++i){
    if (textures[i]->getFilename() == texname){
      tex = textures[i];
      break;
    }
  }
  //load texture
  if (!tex){
    tex = System::Engine::instance()->getRenderer()->createTexture(texname);
    scene_.addTexture(tex);
  }
  return tex;
}

short Compiler::getLengthInfo(const Vector2D& pos, Direction d, Matrix* rot){
  bool longRight = false;
  bool longLeft = false;
  if (d == TOP){
    if (rot)
      *rot = Math::Matrix(Matrix::Identity);
    if (!isWall(pos.x+1,pos.y, TOP) && (isWall(pos.x+1,pos.y, LEFT) || isWall(pos.x+1,pos.y-1, LEFT)))
      longRight = true;
    if (!isWall(pos.x-1,pos.y, TOP) && (isWall(pos.x-1,pos.y, RIGHT) || isWall(pos.x-1,pos.y-1, RIGHT)))
      longLeft = true;
  }
  else if (d == RIGHT){
    //std::cerr << pos.x << " " << pos.y << " " << isWall(pos.x,pos.y-1, RIGHT) << isWall(pos.x,pos.y-1, BOTTOM) << isWall(pos.x+1,pos.y-1, BOTTOM) << "\n";
    if (rot)
      *rot = Math::Matrix(Matrix::Rotation, Vector3D(0,1,0), (float)(270.0/180.0*M_PI));
    if (!isWall(pos.x,pos.y+1, RIGHT) && (isWall(pos.x,pos.y+1, TOP) || isWall(pos.x+1,pos.y+1, TOP)))
      longRight = true;
    if (!isWall(pos.x,pos.y-1, RIGHT) && (isWall(pos.x,pos.y-1, BOTTOM) || isWall(pos.x+1,pos.y-1, BOTTOM)))
      longLeft = true;
  }
  else if (d == BOTTOM){
    if (rot)
      *rot = Math::Matrix(Matrix::Rotation, Vector3D(0,1,0), (float)(180.0/180.0*M_PI));
    if (!isWall(pos.x-1,pos.y, BOTTOM) && (isWall(pos.x-1,pos.y+1, RIGHT) || isWall(pos.x-1,pos.y, RIGHT)))
      longRight = true;
    if (!isWall(pos.x+1,pos.y, BOTTOM) && (isWall(pos.x+1,pos.y+1, LEFT) || isWall(pos.x+1,pos.y, LEFT)))
      longLeft = true;
  }
  else if (d == LEFT){
    if (rot)
      *rot = Math::Matrix(Matrix::Rotation, Vector3D(0,1,0), (float)(90.0/180.0*M_PI));
    if (!isWall(pos.x,pos.y-1, LEFT) && (isWall(pos.x-1,pos.y-1, BOTTOM) || isWall(pos.x,pos.y-1, BOTTOM)))
      longRight = true;
    if (!isWall(pos.x,pos.y+1, LEFT) && (isWall(pos.x-1,pos.y+1, TOP) || isWall(pos.x,pos.y+1, TOP)))
      longLeft = true;
  }

  if (longRight && longLeft)
    return 3;
  else if (longRight)
    return 2;
  else if (longLeft)
    return 1;
  else
    return 0;
}
