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

#include <iostream>
#include <fstream>

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
  Math::Matrix rot = Math::Matrix(Matrix::Identity);
  
  bool longRight = false;
  bool longLeft = false;
  if (d == TOP){
    if (!isWall(pos.x+1,pos.y, TOP) && (isWall(pos.x+1,pos.y, LEFT) || isWall(pos.x+1,pos.y-1, LEFT)))
      longRight = true;
    if (!isWall(pos.x-1,pos.y, TOP) && (isWall(pos.x-1,pos.y, RIGHT) || isWall(pos.x-1,pos.y-1, RIGHT)))
      longLeft = true;
  }
  else if (d == RIGHT){
    //std::cerr << pos.x << " " << pos.y << " " << isWall(pos.x,pos.y-1, RIGHT) << isWall(pos.x,pos.y-1, BOTTOM) << isWall(pos.x+1,pos.y-1, BOTTOM) << "\n";
    rot = Math::Matrix(Matrix::Rotation, Vector3D(0,1,0), 270.0/180.0*M_PI);
    if (!isWall(pos.x,pos.y+1, RIGHT) && (isWall(pos.x,pos.y+1, TOP) || isWall(pos.x+1,pos.y+1, TOP)))
      longRight = true;
    if (!isWall(pos.x,pos.y-1, RIGHT) && (isWall(pos.x,pos.y-1, BOTTOM) || isWall(pos.x+1,pos.y-1, BOTTOM)))
      longLeft = true;
  }
  else if (d == BOTTOM){
    rot = Math::Matrix(Matrix::Rotation, Vector3D(0,1,0), 180.0/180.0*M_PI);
    if (!isWall(pos.x-1,pos.y, BOTTOM) && (isWall(pos.x-1,pos.y+1, RIGHT) || isWall(pos.x-1,pos.y, RIGHT)))
      longRight = true;
    if (!isWall(pos.x+1,pos.y, BOTTOM) && (isWall(pos.x+1,pos.y+1, LEFT) || isWall(pos.x+1,pos.y, LEFT)))
      longLeft = true;
  }
  else if (d == LEFT){
    rot = Math::Matrix(Matrix::Rotation, Vector3D(0,1,0), 90.0/180.0*M_PI);
    if (!isWall(pos.x,pos.y-1, LEFT) && (isWall(pos.x-1,pos.y-1, BOTTOM) || isWall(pos.x,pos.y-1, BOTTOM)))
      longRight = true;
    if (!isWall(pos.x,pos.y+1, LEFT) && (isWall(pos.x-1,pos.y+1, TOP) || isWall(pos.x,pos.y+1, TOP)))
      longLeft = true;
  }

  if (longRight && longLeft)
    tilename = "wallRlLl.obj";
  else if (longRight)
    tilename = "wallRl.obj";
  else if (longLeft)
    tilename = "wallLl.obj";
  
   //see if the wall tile is in there
  MeshGeo::Mesh* msh = NULL;
  std::vector<MeshGeo::Mesh*> meshes = scene_.getMeshes();
  for (unsigned  i = 0; i < meshes.size(); ++i){
    if (meshes[i]->getName() == tilename){
      msh = meshes[i];
      break;
    }
  }
  //load wall tile
  if (!msh){
    msh = new MeshGeo::Mesh();
    msh->loadFromFile("models/world/"+tilename);
    scene_.addMesh(msh);
  }
  //build texture name
  std::string texName;
  texName = "textures/world/floorWall.jpg";
  //see if the texture is already loaded
  Graphics::Texture* tex = NULL;
  std::vector<Graphics::Texture*> textures = scene_.getTextures();
  for (unsigned i = 0; i < textures.size(); ++i){
    if (textures[i]->getFilename() == texName){
      tex = textures[i];
      break;
    }
  }
  //load texture
  if (!tex){
    tex = System::Engine::instance()->getRenderer()->createTexture(texName);
    scene_.addTexture(tex);
  }
  MeshGeo::Model* mdl = new MeshGeo::Model(msh);
  mdl->assignTexture(tex, 0);
  //wall
  mdl->setAttrib(0, 1001);
  //mdl->setAttrib(1, id);
  Math::Matrix mat(Matrix::Translation, Vector3D(pos.x*8+4,0,pos.y*8+4));
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
  MeshGeo::Mesh* msh = NULL;
  std::vector<MeshGeo::Mesh*> meshes = scene_.getMeshes();
  for (unsigned  i = 0; i < meshes.size(); ++i){
    if (meshes[i]->getName() == "ground.obj"){
      msh = meshes[i];
      break;
    }
  }
  //load ground tile
  if (!msh){
    msh = new MeshGeo::Mesh();
    msh->loadFromFile("models/world/ground.obj");
    scene_.addMesh(msh);
    //msh->buildVBO();
  }
  //build texture name
  std::string texName;
  if (id == 0)
    texName = "textures/world/floor.bmp";
  else
    texName = "textures/world/room"+toStr(id)+".bmp";
  //see if the texture is already loaded
  Graphics::Texture* tex = NULL;
  std::vector<Graphics::Texture*> textures = scene_.getTextures();
  for (unsigned i = 0; i < textures.size(); ++i){
    if (textures[i]->getFilename() == texName){
      tex = textures[i];
      break;
    }
  }
  //load texture
  if (!tex){
    tex = System::Engine::instance()->getRenderer()->createTexture(texName);
    scene_.addTexture(tex);
  }
  MeshGeo::Model* mdl = new MeshGeo::Model(msh);
  mdl->assignTexture(tex, 0);
  //ground
  mdl->setAttrib(0, 1000);
  mdl->setAttrib(1, id);
  Math::Matrix mat(Matrix::Translation, Vector3D(pos.x*8+4,0,pos.y*8+4));
  mdl->setTrafo(mat);
  scene_.addModel(mdl);
}

//add hero starting position
void Compiler::addStartPos(const Vector2D& pos){
  //starts_.push_back(pos);
}

//add a monster
void Compiler::addMonster(const Vector2D& pos, char monster[2]){
	monsterPos mp;
	mp.monster[0] = monster[0];
	mp.monster[1] = monster[1];
	mp.monster[2] = '\0';
	mp.pos = pos;
	monsters_.push_back(mp);
}

//add furniture
void Compiler::addFurniture(const Vector2D& pos, char furniture[2]){
	furniturePos fp;
	fp.furniture[0] = furniture[0];
	fp.furniture[1] = furniture[1];
	fp.furniture[2] = '\0';
	fp.pos = pos;
	furnitures_.push_back(fp);
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
