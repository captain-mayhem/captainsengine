//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// | Graphics Programming and Applications (Assignment 5)             |
// |                                                                  |
// | Marco Staginski                                                  |
// | staginski@gmx.de                                                 |
// |------------------------------------------------------------------|
// | File: main.cpp                                                   |
//  ==================================================================

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#ifdef WIN32
#include <direct.h>
#endif
#include "compiler.hh"
 
using std::cerr;
using std::endl;
using std::cout;
using std::string;
using std::ifstream;
using std::ios;

string path;

//getcwd wrapper
const string getCwd(){
  char cwd[2048];
#ifdef WIN32
  _getcwd(cwd, 2048);
#else
  getcwd(cwd, 2048);
#endif
  return string(cwd);
}

//the program starts here
int main(int argc, char* argv[]){
  if (argc < 2){
    cerr << "Usage: HQEdit-Linux <mapfile>" << endl;
    exit(EXIT_FAILURE);
  }
  
  //calculate correct directory
  string temp_path(argv[0]);
  int t = temp_path.find_last_of('/');
  if (t != -1){
    temp_path.erase(t);
    path = getCwd()+"/"+temp_path+"/";
  }
  else
    path = getCwd()+"/";
  
  for (int mc = 1; mc < argc; mc++){
    
  //open .map file
  string temp;
  ifstream in(argv[mc]);
  if (!(in >> temp)){
    cerr << "map not found" << endl;
    exit(EXIT_FAILURE);
  }

  int width,height,i,j;
  char c;
  //read map size (Mapsize: i x j \n);
  in >> width;
  in >> c;
  in >> height;

  //move to correct starting position
  char line[8];
  in.getline(line, 8);
  
  //create the map compiler
  Compiler cmp(width,height);

  //build character representation of the map
  char** map;
  map = new char*[height*3+1];
  for (int k = 0; k < height*3+1; k++){
    map[k] = new char[width*5+2];
    in.getline(map[k], width*5+2);
  }
  
  //Now parse the map data
  for (j = 0; j < height; j++){
    //***********
    //upper wall line
    //***********
    short ypos = j*3;
    i = -1;
    for(short xpos = 2; xpos < width*5; xpos+=5){
      i++;
      c = map[ypos][xpos];
      //is it a horizontal wall?
      if (c == '-'){
        cmp.setWall(Vector2D(i,j), TOP);
      }
      //or a door
      else if (c == 'D'){
        cmp.setDoor(Vector2D(i,j), TOP, 0);
      }
      else if (c == 'S'){
        cmp.setDoor(Vector2D(i,j), TOP, 1);
      }
      else if (c == 'H'){
        cmp.setDoor(Vector2D(i,j), TOP, 2);
      }
    }

    //********************
    //object/action line
    //********************
    ypos++;
    i = -1;
    for(short xpos = 0; xpos < width*5; xpos+=5){
      i++;
      c = map[ypos][xpos];
      //left side
      //is it a horizontal wall?
      if (c == '|'){
        cmp.setWall(Vector2D(i,j), LEFT);
      }
      //or a door?
      else if (c == 'D'){
        cmp.setDoor(Vector2D(i,j), LEFT, 0);
      }
      else if (c == 'S'){
        cmp.setDoor(Vector2D(i,j), LEFT, 1);
      }
      else if (c == 'H'){
        cmp.setDoor(Vector2D(i,j), LEFT, 2);
      }
    }
    
    i = -1;
    for(short xpos = 5; xpos <= width*5; xpos+=5){
      i++;
      c = map[ypos][xpos];
      //right side
      //the same as above
      if (c == '|'){
        cmp.setWall(Vector2D(i,j), RIGHT);
      }
      else if (c == 'D'){
        cmp.setDoor(Vector2D(i,j), RIGHT, 0);
      }
      else if (c == 'S'){
        cmp.setDoor(Vector2D(i,j), RIGHT, 1);
      }
      else if (c == 'H'){
        cmp.setDoor(Vector2D(i,j), RIGHT, 2);
      }
    }

    //monsters and furniture
		i=-1;
		for(short xpos = 1; xpos <=width*5; xpos+=5){
			i++;
			c = map[ypos][xpos];
			if(c == 'M'){
				char monster[] = {map[ypos][xpos+1], map[ypos][xpos+2]};
				cmp.addMonster(Vector2D(i,j), monster);
			}
			else if(c == 'F'){
				char furniture[] = {map[ypos][xpos+1], map[ypos][xpos+2]};
				cmp.addFurniture(Vector2D(i,j), furniture);
			}
		}
		
		//special actions
    i = -1;
    for(short xpos = 4; xpos <= width*5; xpos+=5){
      i++;
      c = map[ypos][xpos];
      //hero starting position?
      if (c == 'X'){
        cmp.addStartPos(Vector2D(i,j));
      }
      else if (c != ' '){
        cmp.addScript(c, Vector2D(i,j));
      }
    }

    //*******************
    //non-blocking object/room line
    //*******************
    ypos++;
    //roomId
    i = -1;
    for(short xpos = 3; xpos < width*5; xpos+=5){
      i++;
      char room[3];
      room[2] = '\0';
      room[0] = map[ypos][xpos];
      //if no id is given, it's 0 (corridor)
      if (room[0] == ' '){
        short id = 0;
        cmp.addRoom(Vector2D(i,j), id);
      }
      //we have a room, so add it
      else{
        room[1] = map[ypos][xpos+1];
        short id = toInt(string(room));
        cmp.addRoom(Vector2D(i,j), id);
      }
    }

    //overlay
    i = -1;
    for(short xpos = 1; xpos <=width*5; xpos+=5){
			i++;
			char overlay[] = {map[ypos][xpos], map[ypos][xpos+1], '\0'};
      if (overlay[0] == '*' || overlay[0] == ' ')
          continue;
			cmp.addOverlay(Vector2D(i,j), overlay);
		}

    
    //******************
    //bottom wall line
    //******************
    ypos++;
    i = -1;
    for(short xpos = 2; xpos < width*5; xpos+=5){
      i++;
      c = map[ypos][xpos];
      //is there a wall?
      if (c == '-'){
        cmp.setWall(Vector2D(i,j), BOTTOM);
      }
      //or a door?
      else if (c == 'D'){
        cmp.setDoor(Vector2D(i,j), BOTTOM, 0);
      }
      else if (c == 'S'){
        cmp.setDoor(Vector2D(i,j), BOTTOM, 1);
      }
      else if (c == 'H'){
        cmp.setDoor(Vector2D(i,j), BOTTOM, 2);
      }
    }
  }
  
  in.close();

  //write the compiled map
  cmp.write(string(argv[mc]));

  //delete allocated memory
  for (int k = 0; k < height; k++){
    delete [] map[k];
  }
  delete [] map;

  }
    
  return 0;
}
