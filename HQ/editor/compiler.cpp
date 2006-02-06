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
#include "../heroquest/common.hh"
#include "compiler.hh"

using std::ofstream;
using std::ios;
using std::cerr;
using std::cout;
using std::endl;

extern string path;

//Constructor, takes map size
Compiler::Compiler(short width, short height){
  width_ = width;
  height_ = height;
  map_ = new Field*[height_];
  for (int j = 0; j < height_; j++){
    map_[j] = new Field[width_];
  }
}

//Destructor
Compiler::~Compiler(){
  //print out map on exit
  for(int j = 0; j < height_; j++){
    for (int i = 0; i < width_; i++){
      int tmp = map_[j][i].getId();
      if (tmp < 10){
       cout << 0;
      }
      cout << tmp <<" ";
    }
    cout << endl;
  }
  //number of monsters
  cout << "Number of Monsters in the map: " << monsters_.size() << endl;
  cout << "Number of Furnitures in the map: " << furnitures_.size() << endl;
  cout << "Number of Overlays in the map: " << overlays_.size() << endl;
  cout << "Number of Scripts in the map: " << scripts_.size() << endl;
  
  //deallocate memory
  for (int j = 0; j < height_; j++){
    delete [] map_[j];
  }
  delete [] map_;
}

//Set a wall
void Compiler::setWall(const Vector2D& pos, Direction d){
  map_[pos.y][pos.x].wallbits.set(d);
}

//is it a wall?
bool Compiler::isWall(short x, short y, Direction dir) const{
  if (x < 0 || x >= width_)
    return false;
  if (y < 0 || y >= height_)
    return false;
  Field& f = map_[y][x];
  return f.wallbits.test(dir);
}

//is it a door?
bool Compiler::isDoor(short x, short y, Direction dir) const{
  if (x < 0 || x >= width_)
    return false;
  if (y < 0 || y >= height_)
    return false;
  Field& f = map_[y][x];
  return f.doorbits.test(dir);
}
	
//set a door
void Compiler::setDoor(const Vector2D& pos, Direction d, bool isSecret){
  map_[pos.y][pos.x].doorbits.set(d);
  //see, if door is already stored, because each door is parsed two times (lower line of one field and upper line of the
  //other field)
  int idx = -1;
  for (unsigned i = 0; i < doors_.size(); i++){
    Door actDoor = doors_[i];
    if (d == TOP){
      if (actDoor.getPosition2() == pos && actDoor.getPosition() == Vector2D(pos.x,pos.y-1)){
        idx = i;
	      break;
      }
    }
    else if (d == RIGHT){
      if (actDoor.getPosition() == pos && actDoor.getPosition2() == Vector2D(pos.x+1,pos.y)){
        idx = i;
	      break;
      }
    }
    else if (d == BOTTOM){
      if (actDoor.getPosition() == pos && actDoor.getPosition2() == Vector2D(pos.x,pos.y+1)){
        idx = i;
        break;
      }
    }
    else if (d == LEFT){
      if (actDoor.getPosition2() == pos && actDoor.getPosition() == Vector2D(pos.x-1,pos.y)){
        idx = i;
	      break;
      }
    }
  }
  //door is not yet there
  if (idx == -1){
    Door newDoor;
    //set the door position correctly
    if (d == TOP){
      newDoor.setPosition(Vector2D(pos.x,pos.y-1));
      newDoor.setPosition2(pos);
    }
    if (d == RIGHT){
      newDoor.setPosition(pos);
      newDoor.setPosition2(Vector2D(pos.x+1,pos.y));
    }
    if (d == BOTTOM){
      newDoor.setPosition(pos);
      newDoor.setPosition2(Vector2D(pos.x,pos.y+1));
    }
    if (d == LEFT){
      newDoor.setPosition(Vector2D(pos.x-1,pos.y));
      newDoor.setPosition2(pos);
    }
    //the door is closed in the beginning
    newDoor.setActive();
    //the door type is stored in the id
    if (isSecret){
      newDoor.setId(1);
    }
    else
      newDoor.setId(0);
    //save door and get index
    doors_.push_back(newDoor);
    idx = doors_.size()-1; 
  }
  //store index in the corresponding Field
  map_[pos.y][pos.x].dooridx[d] = idx;
}

//write the map as .hqm file
void Compiler::write(string filename){
  string out_filename;
  //set correct name and open stream
  int t = filename.find_last_of('.');
  if (t != -1){
    filename.erase(t);
  }
  out_filename = filename + ".hqm";
  ofstream out(out_filename.c_str(), ios::binary);

  //Map Version
  float version = VERSION;
  
  //write header data
  out.write((char*)&version, sizeof(float));
  out.write((char*)&width_, sizeof(short));
  out.write((char*)&height_, sizeof(short));
  //for each field
  for (int j = 0; j < height_; j++){
    for (int i = 0; i < width_; i++){
      //write field content
      Field f = map_[j][i];
      char bits = f.wallbits.getData();
      out.write(&bits, sizeof(char));
      Vector2D pos = f.getPosition();
      out.write((char*)&pos, sizeof(Vector2D));
      short id = f.getId();
      out.write((char*)&id, sizeof(short));
      bool active = f.getStatus();
      out.write((char*)&active, sizeof(bool));
      bits = f.doorbits.getData();
      out.write(&bits, sizeof(char));
      out.write((char*)f.dooridx, 4*sizeof(short));
      //generate 3D geometry
      //this is somehow ugly code
      int counter = 1;
      //walls, they have one quad
      if(f.wallbits.test(TOP))
        counter++;
      if(f.wallbits.test(BOTTOM))
        counter++;
      if(f.wallbits.test(LEFT))
        counter++;
      if(f.wallbits.test(RIGHT))
        counter++;
      //doors, they have 6 quads
      if(f.doorbits.test(TOP))
        counter+=6;
      if(f.doorbits.test(BOTTOM))
        counter+=6;
      if(f.doorbits.test(LEFT))
        counter+=6;
      if(f.doorbits.test(RIGHT))
        counter+=6;
      
      //a quad has 4 vertices
      f.numVertices = 4*counter;
      //vertices and texture coordinates
      f.vertices = new Vector3D[f.numVertices];
      f.texCoords = new Vec2f[f.numVertices];
      counter = 0;
      //ground tile
      f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE, 0, j*QUADSIZE         );
      f.texCoords[counter++] = Vec2f(1,1);
      f.vertices[counter] = Vector3D(i*QUADSIZE         , 0, j*QUADSIZE         );
      f.texCoords[counter++] = Vec2f(0,1);
      f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE, 0, j*QUADSIZE+QUADSIZE);
      f.texCoords[counter++] = Vec2f(1,0);
      f.vertices[counter] = Vector3D(i*QUADSIZE         , 0, j*QUADSIZE+QUADSIZE);
      f.texCoords[counter++] = Vec2f(0,0);
      //top walls
      if (f.wallbits.test(TOP)){
        int LEFTTHICK = -THICKNESS;
	      int RIGHTTHICK = THICKNESS;
	      if (isWall(i-1, j, TOP) || isDoor(i-1, j, TOP)){
	        LEFTTHICK = 0;
	      }
	      if (isWall(i+1, j, TOP) || isDoor(i+1, j, TOP)){
	        RIGHTTHICK = 0;
	      }
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE+RIGHTTHICK, WALLHEIGHT, j*QUADSIZE+THICKNESS);
        f.texCoords[counter++] = Vec2f(1,2);
        f.vertices[counter] = Vector3D(i*QUADSIZE+LEFTTHICK, WALLHEIGHT, j*QUADSIZE+THICKNESS);
        f.texCoords[counter++] = Vec2f(0,2);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE+RIGHTTHICK, 0, j*QUADSIZE+THICKNESS);
        f.texCoords[counter++] = Vec2f(1,0);
        f.vertices[counter] = Vector3D(i*QUADSIZE+LEFTTHICK, 0, j*QUADSIZE+THICKNESS);
        f.texCoords[counter++] = Vec2f(0,0);
      }
      //right walls
      if (f.wallbits.test(RIGHT)){
        int LEFTTHICK = -THICKNESS;
	      int RIGHTTHICK = THICKNESS;
	      if (isWall(i, j-1, RIGHT) || isDoor(i,j-1,RIGHT)){
	        LEFTTHICK = 0;
	      }
	      if (isWall(i, j+1, RIGHT) || isDoor(i, j+1, RIGHT)){
	        RIGHTTHICK = 0;
	      }
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-THICKNESS, WALLHEIGHT, j*QUADSIZE+QUADSIZE+RIGHTTHICK);
        f.texCoords[counter++] = Vec2f(1,2);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-THICKNESS, WALLHEIGHT, j*QUADSIZE+LEFTTHICK);
        f.texCoords[counter++] = Vec2f(0,2);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-THICKNESS, 0, j*QUADSIZE+QUADSIZE+RIGHTTHICK);
        f.texCoords[counter++] = Vec2f(1,0);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-THICKNESS, 0, j*QUADSIZE+LEFTTHICK);
        f.texCoords[counter++] = Vec2f(0,0);
      }
      //bottom walls
      if (f.wallbits.test(BOTTOM)){
        int LEFTTHICK = THICKNESS;
	      int RIGHTTHICK = -THICKNESS;
	      if (isWall(i+1, j, BOTTOM) || isDoor(i+1,j, BOTTOM)){
	        LEFTTHICK = 0;
	      }
	      if (isWall(i-1, j, BOTTOM) || isDoor(i-1, j, BOTTOM)){
	        RIGHTTHICK = 0;
	      }
        f.vertices[counter] = Vector3D(i*QUADSIZE+RIGHTTHICK, WALLHEIGHT, j*QUADSIZE+QUADSIZE-THICKNESS);
        f.texCoords[counter++] = Vec2f(1,2);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE+LEFTTHICK, WALLHEIGHT, j*QUADSIZE+QUADSIZE-THICKNESS);
        f.texCoords[counter++] = Vec2f(0,2);
        f.vertices[counter] = Vector3D(i*QUADSIZE+RIGHTTHICK, 0, j*QUADSIZE+QUADSIZE-THICKNESS);
        f.texCoords[counter++] = Vec2f(1,0);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE+LEFTTHICK, 0, j*QUADSIZE+QUADSIZE-THICKNESS);
        f.texCoords[counter++] = Vec2f(0,0);
      }
      //left walls
      if (f.wallbits.test(LEFT)){
        int LEFTTHICK = THICKNESS;
	      int RIGHTTHICK = -THICKNESS;
	      if (isWall(i, j+1, LEFT) || isDoor(i, j+1, LEFT)){
	        LEFTTHICK = 0;
	      }
	      if (isWall(i, j-1, LEFT) || isDoor(i, j-1, LEFT)){
	        RIGHTTHICK = 0;
	      }
        f.vertices[counter] = Vector3D(i*QUADSIZE+THICKNESS, WALLHEIGHT, j*QUADSIZE+RIGHTTHICK);
        f.texCoords[counter++] = Vec2f(1,2);
        f.vertices[counter] = Vector3D(i*QUADSIZE+THICKNESS, WALLHEIGHT, j*QUADSIZE+QUADSIZE+LEFTTHICK);
        f.texCoords[counter++] = Vec2f(0,2);
        f.vertices[counter] = Vector3D(i*QUADSIZE+THICKNESS, 0, j*QUADSIZE+RIGHTTHICK);
        f.texCoords[counter++] = Vec2f(1,0);
        f.vertices[counter] = Vector3D(i*QUADSIZE+THICKNESS, 0, j*QUADSIZE+QUADSIZE+LEFTTHICK);
        f.texCoords[counter++] = Vec2f(0,0);
      }
      //top doors
      if (f.doorbits.test(TOP)){
        int LEFTTHICK = -THICKNESS;
	      int RIGHTTHICK = THICKNESS;
	      if (isWall(i-1, j, TOP) || isDoor(i-1, j, TOP)){
	        LEFTTHICK = 0;
	      }
	      if (isWall(i+1, j, TOP) || isDoor(i+1, j, TOP)){
	        RIGHTTHICK = 0;
	      }
	      //upper part
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE+RIGHTTHICK, WALLHEIGHT, j*QUADSIZE+THICKNESS);
        f.texCoords[counter++] = Vec2f(1,2);
        f.vertices[counter] = Vector3D(i*QUADSIZE+LEFTTHICK, WALLHEIGHT, j*QUADSIZE+THICKNESS);
        f.texCoords[counter++] = Vec2f(0,2);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE+RIGHTTHICK, DOORHEIGHT, j*QUADSIZE+THICKNESS);
        f.texCoords[counter++] = Vec2f(1,DOORTEX);
        f.vertices[counter] = Vector3D(i*QUADSIZE+LEFTTHICK, DOORHEIGHT, j*QUADSIZE+THICKNESS);
        f.texCoords[counter++] = Vec2f(0,DOORTEX);
	      //left part
        f.vertices[counter] = Vector3D(i*QUADSIZE+THICKNESS, DOORHEIGHT, j*QUADSIZE+THICKNESS);
        f.texCoords[counter++] = Vec2f((float)(THICKNESS-LEFTTHICK)/QUADSIZE,DOORTEX);
        f.vertices[counter] = Vector3D(i*QUADSIZE+LEFTTHICK, DOORHEIGHT, j*QUADSIZE+THICKNESS);
        f.texCoords[counter++] = Vec2f(0,DOORTEX);
        f.vertices[counter] = Vector3D(i*QUADSIZE+THICKNESS, 0, j*QUADSIZE+THICKNESS);
        f.texCoords[counter++] = Vec2f((float)(THICKNESS-LEFTTHICK)/QUADSIZE,0);
        f.vertices[counter] = Vector3D(i*QUADSIZE+LEFTTHICK, 0, j*QUADSIZE+THICKNESS);
        f.texCoords[counter++] = Vec2f(0,0);
	      //right part
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE+RIGHTTHICK, DOORHEIGHT, j*QUADSIZE+THICKNESS);
        f.texCoords[counter++] = Vec2f(1,DOORTEX);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-THICKNESS, DOORHEIGHT, j*QUADSIZE+THICKNESS);
        f.texCoords[counter++] = Vec2f((float)(QUADSIZE-THICKNESS-RIGHTTHICK)/QUADSIZE,DOORTEX);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE+RIGHTTHICK, 0, j*QUADSIZE+THICKNESS);
        f.texCoords[counter++] = Vec2f(1,0);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-THICKNESS, 0, j*QUADSIZE+THICKNESS);
        f.texCoords[counter++] = Vec2f((float)(QUADSIZE-THICKNESS-RIGHTTHICK)/QUADSIZE,0);
	      //left inner part
	      f.vertices[counter] = Vector3D(i*QUADSIZE+DOORTHICK, DOORHEIGHT, j*QUADSIZE);
	      f.texCoords[counter++] = Vec2f((float)THICKNESS/QUADSIZE,DOORTEX);
        f.vertices[counter] = Vector3D(i*QUADSIZE+DOORTHICK, DOORHEIGHT, j*QUADSIZE+THICKNESS);
	      f.texCoords[counter++] = Vec2f(0,DOORTEX);
	      f.vertices[counter] = Vector3D(i*QUADSIZE+DOORTHICK, 0, j*QUADSIZE);
	      f.texCoords[counter++] = Vec2f((float)THICKNESS/QUADSIZE,0);
	      f.vertices[counter] = Vector3D(i*QUADSIZE+DOORTHICK, 0, j*QUADSIZE+THICKNESS);
	      f.texCoords[counter++] = Vec2f(0,0);
	      //right inner part
	      f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-DOORTHICK, DOORHEIGHT, j*QUADSIZE+THICKNESS);
	      f.texCoords[counter++] = Vec2f((float)THICKNESS/QUADSIZE,DOORTEX);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-DOORTHICK, DOORHEIGHT, j*QUADSIZE);
	      f.texCoords[counter++] = Vec2f(0,DOORTEX);
	      f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-DOORTHICK, 0, j*QUADSIZE+THICKNESS);
	      f.texCoords[counter++] = Vec2f((float)THICKNESS/QUADSIZE,0);
	      f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-DOORTHICK, 0, j*QUADSIZE);
	      f.texCoords[counter++] = Vec2f(0,0);
	      //upper inner part
	      f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-DOORTHICK, DOORHEIGHT, j*QUADSIZE+THICKNESS);
	      f.texCoords[counter++] = Vec2f(1-(float)THICKNESS/QUADSIZE,(float)THICKNESS/QUADSIZE);
        f.vertices[counter] = Vector3D(i*QUADSIZE+DOORTHICK, DOORHEIGHT, j*QUADSIZE+THICKNESS);
	      f.texCoords[counter++] = Vec2f((float)THICKNESS/QUADSIZE,(float)THICKNESS/QUADSIZE);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-DOORTHICK, DOORHEIGHT, j*QUADSIZE);
	      f.texCoords[counter++] = Vec2f(1-(float)THICKNESS/QUADSIZE,0);
	      f.vertices[counter] = Vector3D(i*QUADSIZE+DOORTHICK, DOORHEIGHT, j*QUADSIZE);
	      f.texCoords[counter++] = Vec2f((float)THICKNESS/QUADSIZE,0);
	
      }
       //right doors
      if (f.doorbits.test(RIGHT)){
        int LEFTTHICK = -THICKNESS;
	      int RIGHTTHICK = THICKNESS;
	      if (isWall(i, j-1, RIGHT) || isDoor(i,j-1,RIGHT)){
	        LEFTTHICK = 0;
	      }
	      if (isWall(i, j+1, RIGHT) || isDoor(i, j+1, RIGHT)){
	        RIGHTTHICK = 0;
	      }
	      //upper part
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-THICKNESS, WALLHEIGHT, j*QUADSIZE+QUADSIZE+RIGHTTHICK);
        f.texCoords[counter++] = Vec2f(1,2);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-THICKNESS, WALLHEIGHT, j*QUADSIZE+LEFTTHICK);
        f.texCoords[counter++] = Vec2f(0,2);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-THICKNESS, DOORHEIGHT, j*QUADSIZE+QUADSIZE+RIGHTTHICK);
        f.texCoords[counter++] = Vec2f(1,DOORTEX);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-THICKNESS, DOORHEIGHT, j*QUADSIZE+LEFTTHICK);
        f.texCoords[counter++] = Vec2f(0,DOORTEX);
	      //left part
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-THICKNESS, DOORHEIGHT, j*QUADSIZE+THICKNESS);
        f.texCoords[counter++] = Vec2f((float)(THICKNESS-LEFTTHICK)/QUADSIZE,DOORTEX);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-THICKNESS, DOORHEIGHT, j*QUADSIZE+LEFTTHICK);
        f.texCoords[counter++] = Vec2f(0,DOORTEX);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-THICKNESS, 0, j*QUADSIZE+THICKNESS);
        f.texCoords[counter++] = Vec2f((float)(THICKNESS-LEFTTHICK)/QUADSIZE,0);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-THICKNESS, 0, j*QUADSIZE+LEFTTHICK);
        f.texCoords[counter++] = Vec2f(0,0);
	      //right part
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-THICKNESS, DOORHEIGHT, j*QUADSIZE+QUADSIZE+RIGHTTHICK);
        f.texCoords[counter++] = Vec2f(1,DOORTEX);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-THICKNESS, DOORHEIGHT, j*QUADSIZE+QUADSIZE-THICKNESS);
        f.texCoords[counter++] = Vec2f((float)(QUADSIZE-THICKNESS-RIGHTTHICK)/QUADSIZE,DOORTEX);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-THICKNESS, 0, j*QUADSIZE+QUADSIZE+RIGHTTHICK);
        f.texCoords[counter++] = Vec2f(1,0);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-THICKNESS, 0, j*QUADSIZE+QUADSIZE-THICKNESS);
        f.texCoords[counter++] = Vec2f((float)(QUADSIZE-THICKNESS-RIGHTTHICK)/QUADSIZE,0);
	      //left inner part
	      f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE, DOORHEIGHT, j*QUADSIZE+DOORTHICK);
	      f.texCoords[counter++] = Vec2f((float)THICKNESS/QUADSIZE,DOORTEX);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-THICKNESS, DOORHEIGHT, j*QUADSIZE+DOORTHICK);
	      f.texCoords[counter++] = Vec2f(0,DOORTEX);
	      f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE, 0, j*QUADSIZE+DOORTHICK);
	      f.texCoords[counter++] = Vec2f((float)THICKNESS/QUADSIZE,0);
	      f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-THICKNESS, 0, j*QUADSIZE+DOORTHICK);
	      f.texCoords[counter++] = Vec2f(0,0);
	      //right inner part
	      f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-THICKNESS, DOORHEIGHT, j*QUADSIZE+QUADSIZE-DOORTHICK);
	      f.texCoords[counter++] = Vec2f((float)THICKNESS/QUADSIZE,DOORTEX);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE, DOORHEIGHT, j*QUADSIZE+QUADSIZE-DOORTHICK);
	      f.texCoords[counter++] = Vec2f(0,DOORTEX);
	      f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-THICKNESS, 0, j*QUADSIZE+QUADSIZE-DOORTHICK);
	      f.texCoords[counter++] = Vec2f((float)THICKNESS/QUADSIZE,0);
	      f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE, 0, j*QUADSIZE+QUADSIZE-DOORTHICK);
	      f.texCoords[counter++] = Vec2f(0,0);
	      //upper inner part
	      f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-THICKNESS, DOORHEIGHT, j*QUADSIZE+QUADSIZE-DOORTHICK);
	      f.texCoords[counter++] = Vec2f(1-(float)THICKNESS/QUADSIZE,(float)THICKNESS/QUADSIZE);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-THICKNESS, DOORHEIGHT, j*QUADSIZE+DOORTHICK);
	      f.texCoords[counter++] = Vec2f((float)THICKNESS/QUADSIZE,(float)THICKNESS/QUADSIZE);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE, DOORHEIGHT, j*QUADSIZE+QUADSIZE-DOORTHICK);
	      f.texCoords[counter++] = Vec2f(1-(float)THICKNESS/QUADSIZE,0);
	      f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE, DOORHEIGHT, j*QUADSIZE+DOORTHICK);
	      f.texCoords[counter++] = Vec2f((float)THICKNESS/QUADSIZE,0);
      }
      //bottom doors
      if (f.doorbits.test(BOTTOM)){
        int LEFTTHICK = THICKNESS;
	      int RIGHTTHICK = -THICKNESS;
	      if (isWall(i+1, j, BOTTOM) || isDoor(i+1,j, BOTTOM)){
	        LEFTTHICK = 0;
	      }
	      if (isWall(i-1, j, BOTTOM) || isDoor(i-1, j, BOTTOM)){
	        RIGHTTHICK = 0;
	      }
	      //upper part
        f.vertices[counter] = Vector3D(i*QUADSIZE+RIGHTTHICK, WALLHEIGHT, j*QUADSIZE+QUADSIZE-THICKNESS);
        f.texCoords[counter++] = Vec2f(1,2);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE+LEFTTHICK, WALLHEIGHT, j*QUADSIZE+QUADSIZE-THICKNESS);
        f.texCoords[counter++] = Vec2f(0,2);
        f.vertices[counter] = Vector3D(i*QUADSIZE+RIGHTTHICK, DOORHEIGHT, j*QUADSIZE+QUADSIZE-THICKNESS);
        f.texCoords[counter++] = Vec2f(1,DOORTEX);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE+LEFTTHICK, DOORHEIGHT, j*QUADSIZE+QUADSIZE-THICKNESS);
        f.texCoords[counter++] = Vec2f(0,DOORTEX);
	      //left part
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-THICKNESS, DOORHEIGHT, j*QUADSIZE+QUADSIZE-THICKNESS);
        f.texCoords[counter++] = Vec2f((float)(THICKNESS+LEFTTHICK)/QUADSIZE,DOORTEX);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE+LEFTTHICK, DOORHEIGHT, j*QUADSIZE+QUADSIZE-THICKNESS);
        f.texCoords[counter++] = Vec2f(0,DOORTEX);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-THICKNESS, 0, j*QUADSIZE+QUADSIZE-THICKNESS);
        f.texCoords[counter++] = Vec2f((float)(THICKNESS+LEFTTHICK)/QUADSIZE,0);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE+LEFTTHICK, 0, j*QUADSIZE+QUADSIZE-THICKNESS);
        f.texCoords[counter++] = Vec2f(0,0);
	      //right part
        f.vertices[counter] = Vector3D(i*QUADSIZE+RIGHTTHICK, DOORHEIGHT, j*QUADSIZE+QUADSIZE-THICKNESS);
        f.texCoords[counter++] = Vec2f(1,DOORTEX);
        f.vertices[counter] = Vector3D(i*QUADSIZE+THICKNESS, DOORHEIGHT, j*QUADSIZE+QUADSIZE-THICKNESS);
        f.texCoords[counter++] = Vec2f((float)(QUADSIZE-THICKNESS+RIGHTTHICK)/QUADSIZE,DOORTEX);
        f.vertices[counter] = Vector3D(i*QUADSIZE+RIGHTTHICK, 0, j*QUADSIZE+QUADSIZE-THICKNESS);
        f.texCoords[counter++] = Vec2f(1,0);
        f.vertices[counter] = Vector3D(i*QUADSIZE+THICKNESS, 0, j*QUADSIZE+QUADSIZE-THICKNESS);
        f.texCoords[counter++] = Vec2f((float)(QUADSIZE-THICKNESS+RIGHTTHICK)/QUADSIZE,0);
	      //left inner part
	      f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-DOORTHICK, DOORHEIGHT, j*QUADSIZE+QUADSIZE);
	      f.texCoords[counter++] = Vec2f((float)THICKNESS/QUADSIZE,DOORTEX);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-DOORTHICK, DOORHEIGHT, j*QUADSIZE+QUADSIZE-THICKNESS);
	      f.texCoords[counter++] = Vec2f(0,DOORTEX);
	      f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-DOORTHICK, 0, j*QUADSIZE+QUADSIZE);
	      f.texCoords[counter++] = Vec2f((float)THICKNESS/QUADSIZE,0);
	      f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-DOORTHICK, 0, j*QUADSIZE+QUADSIZE-THICKNESS);
	      f.texCoords[counter++] = Vec2f(0,0);
	      //right inner part
	      f.vertices[counter] = Vector3D(i*QUADSIZE+DOORTHICK, DOORHEIGHT, j*QUADSIZE+QUADSIZE-THICKNESS);
	      f.texCoords[counter++] = Vec2f((float)THICKNESS/QUADSIZE,DOORTEX);
        f.vertices[counter] = Vector3D(i*QUADSIZE+DOORTHICK, DOORHEIGHT, j*QUADSIZE+QUADSIZE);
	      f.texCoords[counter++] = Vec2f(0,DOORTEX);
	      f.vertices[counter] = Vector3D(i*QUADSIZE+DOORTHICK, 0, j*QUADSIZE+QUADSIZE-THICKNESS);
	      f.texCoords[counter++] = Vec2f((float)THICKNESS/QUADSIZE,0);
	      f.vertices[counter] = Vector3D(i*QUADSIZE+DOORTHICK, 0, j*QUADSIZE+QUADSIZE);
	      f.texCoords[counter++] = Vec2f(0,0);
	      //upper inner part
	      f.vertices[counter] = Vector3D(i*QUADSIZE+DOORTHICK, DOORHEIGHT, j*QUADSIZE+QUADSIZE-THICKNESS);
	      f.texCoords[counter++] = Vec2f(1-(float)THICKNESS/QUADSIZE,(float)THICKNESS/QUADSIZE);
        f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-DOORTHICK, DOORHEIGHT, j*QUADSIZE+QUADSIZE-THICKNESS);
	      f.texCoords[counter++] = Vec2f((float)THICKNESS/QUADSIZE,(float)THICKNESS/QUADSIZE);
        f.vertices[counter] = Vector3D(i*QUADSIZE+DOORTHICK, DOORHEIGHT, j*QUADSIZE+QUADSIZE);
	      f.texCoords[counter++] = Vec2f(1-(float)THICKNESS/QUADSIZE,0);
	      f.vertices[counter] = Vector3D(i*QUADSIZE+QUADSIZE-DOORTHICK, DOORHEIGHT, j*QUADSIZE+QUADSIZE);
	      f.texCoords[counter++] = Vec2f((float)THICKNESS/QUADSIZE,0);
      }
      //left doors
      if (f.doorbits.test(LEFT)){
        int LEFTTHICK = THICKNESS;
	      int RIGHTTHICK = -THICKNESS;
	      if (isWall(i, j+1, LEFT) || isDoor(i, j+1, LEFT)){
	        LEFTTHICK = 0;
	      }
	      if (isWall(i, j-1, LEFT) || isDoor(i, j-1, LEFT)){
	        RIGHTTHICK = 0;
	      }
	      //upper part
        f.vertices[counter] = Vector3D(i*QUADSIZE+THICKNESS, WALLHEIGHT, j*QUADSIZE+RIGHTTHICK);
        f.texCoords[counter++] = Vec2f(1,2);
        f.vertices[counter] = Vector3D(i*QUADSIZE+THICKNESS, WALLHEIGHT, j*QUADSIZE+QUADSIZE+LEFTTHICK);
        f.texCoords[counter++] = Vec2f(0,2);
        f.vertices[counter] = Vector3D(i*QUADSIZE+THICKNESS, DOORHEIGHT, j*QUADSIZE+RIGHTTHICK);
        f.texCoords[counter++] = Vec2f(1,DOORTEX);
        f.vertices[counter] = Vector3D(i*QUADSIZE+THICKNESS, DOORHEIGHT, j*QUADSIZE+QUADSIZE+LEFTTHICK);
        f.texCoords[counter++] = Vec2f(0,DOORTEX);
	      //left part
        f.vertices[counter] = Vector3D(i*QUADSIZE+THICKNESS, DOORHEIGHT, j*QUADSIZE+QUADSIZE-THICKNESS);
        f.texCoords[counter++] = Vec2f((float)(THICKNESS+LEFTTHICK)/QUADSIZE,DOORTEX);
        f.vertices[counter] = Vector3D(i*QUADSIZE+THICKNESS, DOORHEIGHT, j*QUADSIZE+QUADSIZE+LEFTTHICK);
        f.texCoords[counter++] = Vec2f(0,DOORTEX);
        f.vertices[counter] = Vector3D(i*QUADSIZE+THICKNESS, 0, j*QUADSIZE+QUADSIZE-THICKNESS);
        f.texCoords[counter++] = Vec2f((float)(THICKNESS+LEFTTHICK)/QUADSIZE,0);
        f.vertices[counter] = Vector3D(i*QUADSIZE+THICKNESS, 0, j*QUADSIZE+QUADSIZE+LEFTTHICK);
        f.texCoords[counter++] = Vec2f(0,0);
	      //right part
        f.vertices[counter] = Vector3D(i*QUADSIZE+THICKNESS, DOORHEIGHT, j*QUADSIZE+RIGHTTHICK);
        f.texCoords[counter++] = Vec2f(1,DOORTEX);
        f.vertices[counter] = Vector3D(i*QUADSIZE+THICKNESS, DOORHEIGHT, j*QUADSIZE+THICKNESS);
        f.texCoords[counter++] = Vec2f((float)(QUADSIZE-THICKNESS+RIGHTTHICK)/QUADSIZE,DOORTEX);
        f.vertices[counter] = Vector3D(i*QUADSIZE+THICKNESS, 0, j*QUADSIZE+RIGHTTHICK);
        f.texCoords[counter++] = Vec2f(1,0);
        f.vertices[counter] = Vector3D(i*QUADSIZE+THICKNESS, 0, j*QUADSIZE+THICKNESS);
        f.texCoords[counter++] = Vec2f((float)(QUADSIZE-THICKNESS+RIGHTTHICK)/QUADSIZE,0);
	      //left inner part
	      f.vertices[counter] = Vector3D(i*QUADSIZE, DOORHEIGHT, j*QUADSIZE+QUADSIZE-DOORTHICK);
	      f.texCoords[counter++] = Vec2f((float)THICKNESS/QUADSIZE,DOORTEX);
        f.vertices[counter] = Vector3D(i*QUADSIZE+THICKNESS, DOORHEIGHT, j*QUADSIZE+QUADSIZE-DOORTHICK);
	      f.texCoords[counter++] = Vec2f(0,DOORTEX);
	      f.vertices[counter] = Vector3D(i*QUADSIZE, 0, j*QUADSIZE+QUADSIZE-DOORTHICK);
	      f.texCoords[counter++] = Vec2f((float)THICKNESS/QUADSIZE,0);
	      f.vertices[counter] = Vector3D(i*QUADSIZE+THICKNESS, 0, j*QUADSIZE+QUADSIZE-DOORTHICK);
	      f.texCoords[counter++] = Vec2f(0,0);
	      //right inner part
	      f.vertices[counter] = Vector3D(i*QUADSIZE+THICKNESS, DOORHEIGHT, j*QUADSIZE+DOORTHICK);
	      f.texCoords[counter++] = Vec2f((float)THICKNESS/QUADSIZE,DOORTEX);
        f.vertices[counter] = Vector3D(i*QUADSIZE, DOORHEIGHT, j*QUADSIZE+DOORTHICK);
	      f.texCoords[counter++] = Vec2f(0,DOORTEX);
	      f.vertices[counter] = Vector3D(i*QUADSIZE+THICKNESS, 0, j*QUADSIZE+DOORTHICK);
	      f.texCoords[counter++] = Vec2f((float)THICKNESS/QUADSIZE,0);
	      f.vertices[counter] = Vector3D(i*QUADSIZE, 0, j*QUADSIZE+DOORTHICK);
	      f.texCoords[counter++] = Vec2f(0,0);
	      //upper inner part
	      f.vertices[counter] = Vector3D(i*QUADSIZE+THICKNESS, DOORHEIGHT, j*QUADSIZE+DOORTHICK);
	      f.texCoords[counter++] = Vec2f(1-(float)THICKNESS/QUADSIZE,(float)THICKNESS/QUADSIZE);
        f.vertices[counter] = Vector3D(i*QUADSIZE+THICKNESS, DOORHEIGHT, j*QUADSIZE+QUADSIZE-DOORTHICK);
	      f.texCoords[counter++] = Vec2f((float)THICKNESS/QUADSIZE,(float)THICKNESS/QUADSIZE);
        f.vertices[counter] = Vector3D(i*QUADSIZE, DOORHEIGHT, j*QUADSIZE+DOORTHICK);
	      f.texCoords[counter++] = Vec2f(1-(float)THICKNESS/QUADSIZE,0);
	      f.vertices[counter] = Vector3D(i*QUADSIZE, DOORHEIGHT, j*QUADSIZE+QUADSIZE-DOORTHICK);
	      f.texCoords[counter++] = Vec2f((float)THICKNESS/QUADSIZE,0);
      }
      
      //write the generated vertices and textureCoordinates
      out.write((char*)&f.numVertices, sizeof(unsigned));
      for (unsigned k = 0; k < f.numVertices; k++){
	      out.write((char*)&f.texCoords[k], sizeof(Vec2f));
        out.write((char*)&f.vertices[k], sizeof(Vector3D));
      }
    }
  }
  //hero starting positions
  unsigned short size = starts_.size();
  out.write((char*)&size, sizeof(unsigned short));
  for (unsigned i = 0; i < size; i++){
    out.write((char*)&starts_[i], sizeof(Vector2D));
  }

  //room structure
  size = rooms_.size();
  out.write((char*)&size, sizeof(unsigned short));
  for (unsigned i = 0; i< size; i++){
    vector<Vector2D>& temp = rooms_[i];
    unsigned short fields = temp.size();
    out.write((char*)&fields, sizeof(unsigned short));
    for (unsigned j = 0; j < fields; j++){
      Vector2D v = temp[j];
      out.write((char*)&v, sizeof(Vector2D));
    }
  }

  //doors
  size = doors_.size();
  out.write((char*)&size, sizeof(size));
  for (unsigned i = 0; i < size; i++){
    Door d = doors_[i];
    d.write(out);
  }
 
  //monsters
  size = monsters_.size();
  out.write((char*)&size, sizeof(size));
  for (unsigned i = 0; i < size; i++){
  	out.write((char*) &monsters_[i], sizeof(monsters_[i]));
	}

	//furniture
	size = furnitures_.size();
  out.write((char*)&size, sizeof(size));
  for (unsigned i = 0; i < size; i++){
  	out.write((char*) &furnitures_[i], sizeof(furnitures_[i]));
	}

  //overlays
  size = overlays_.size();
  out.write((char*)&size, sizeof(size));
  for (unsigned i = 0; i < size; i++){
    out.write((char*) &overlays_[i], sizeof(overlays_[i]));
  }
  
  //scripts
  size = scripts_.size();
  out.write((char*)&size, sizeof(size));
  for (unsigned i = 0; i < size; i++){
    out.write((char*) &scripts_[i], sizeof(scripts_[i]));
  }

  out.close();
}

//add a field to a room
void Compiler::addRoom(const Vector2D& pos, short id){
  //allocate memory in rooms if room was not yet present
  while (rooms_.size() <= (unsigned short)id){
    vector<Vector2D> tmp;
    rooms_.push_back(tmp);
  }
  //data in rooms
  vector<Vector2D> *room = &rooms_[id];
  room->push_back(pos);
  //position
  map_[pos.y][pos.x].setPosition(pos);
  //id
  map_[pos.y][pos.x].setId(id);
  //not visible in the beginning
  map_[pos.y][pos.x].setActive(false);
}

//add hero starting position
void Compiler::addStartPos(const Vector2D& pos){
  starts_.push_back(pos);
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
