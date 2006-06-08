///  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: world.cpp                                                  |
//  ==================================================================

#ifdef _CLIENT_
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include "renderer/texture.h"
#include "camera.h"
#include "renderer/font.h"
#endif

#ifdef WIN32
#ifdef _DEBUG
#include <windows.h>
#include <crtdbg.h>
#endif
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <assert.h>
#include <ctype.h>

#include "player.h"
#ifdef _CLIENT_
#include "gamestate.h"
#endif
#include "templates.h"
#include "script.h"
#include "world.h"


using std::ifstream;
using std::ostringstream;
using std::ios;

extern string path;

//Constructor
World::World(void) : width_(0), height_(0), world_(0), numberOfVerts_(0), starts_(0){
	loaded_ = false;
	wallCollision_ = true;
  respectClasses_ = true;
}

World::World(const World& w){
	cerr << "World copy constructor not yet complete";
	width_ = w.width_;
	height_ = w.height_;
	loaded_ = w.loaded_;
	//Field** world_;//!
	numberOfVerts_ = w.numberOfVerts_;
	//Vector3D** vWorld_;//!
	//Vector3D moveBox_[4];//!
	heroSize_ = w.heroSize_;
	//Vector2D* starts_;//!
	//Hero* heros_;//!
	monsters_ = vector<Monster*>(w.monsters_);
	furniture_ = vector<Furniture*>(w.furniture_);
	//vector< vector< Field*> > rooms_;//!
	canSee_ = vector<Field*>(w.canSee_);
	visibleFields_ = vector<Field*>(w.visibleFields_);
	doors_ = vector<Door*>(w.doors_);
}

//Destructor
World::~World(void){
	//TODO save destuction needed
  //deInit();
}

//Deallocate world memory
void World::deInit(){
	if (world_ != NULL){
    
    loaded_ = false;
    wait();
    wait();
    wait();
    
    for (int j = 0; j < height_; j++){
      for (int i = 0; i < width_; i++){
        Field* f = &world_[j][i];
        delete [] f->vertices;
        delete [] f->texCoords;
      }
			//TODO why segfault delete [] world_[j];
		}
		delete [] world_;
    
    width_ = 0;
    height_ = 0;

    delete [] starts_;
    delete [] heros_;

    for (unsigned i = 0; i < monsters_.size(); i++){
      delete monsters_[i];
    }
    monsters_.clear();

    for (unsigned i = 0; i < furniture_.size(); i++){
      delete furniture_[i];
    }
    furniture_.clear();

    for (unsigned i = 0; i < overlays_.size(); i++){
      delete overlays_[i];
    }
    overlays_.clear();

    doors_.clear();

    //TODO reset other datastructures
    
#ifdef _CLIENT_
    delete [] vWorld_;
#endif

	}
}

//load the world
bool World::load(const string& name){

	deInit();
  
  float version;
	ifstream in(name.c_str(), ios::binary);
	in.read((char*)&version, sizeof(float));
	in.read((char*)&width_, sizeof(short));
	in.read((char*)&height_, sizeof(short));
	if (version <= 0 || version  > VERSION+0.01){
		in.close();
		return false;
	}

	//allocate memory for the map
	world_ = new Field*[height_];
	for (int j = 0; j < height_; j++){
		world_[j] = new Field[width_];
	}

	//read map data
	for (int j = 0; j < height_; j++){
		for (int i = 0; i < width_; i++){
			//readin field with all its values
			Field* f = &world_[j][i];
			char bits;
			in.read(&bits, sizeof(char));
			f->wallbits.setData(bits);
			Vector2D pos;
			in.read((char*)&pos, sizeof(Vector2D));
			f->setPosition(pos);
			short id;
			in.read((char*)&id, sizeof(short));
			f->setId(id);
			bool active;
			in.read((char*)&active, sizeof(bool));
			f->setActive(active);
			in.read((char*)&bits, sizeof(char));
			f->doorbits.setData(bits);
			in.read((char*)f->dooridx, 4*sizeof(short));
      
			//readin 3D geometry
			in.read((char*)&f->numVertices, sizeof(unsigned));
			f->vertices = new Vector3D[f->numVertices];
			f->texCoords = new Vec2f[f->numVertices];
			numberOfVerts_ += f->numVertices;
			for (unsigned k = 0; k < f->numVertices; k++){
				in.read((char*)&f->texCoords[k], sizeof(Vec2f));
				in.read((char*)&f->vertices[k], sizeof(Vector3D));
			}
		}
	}

	//Starting positions
	in.read((char*)&heroSize_, sizeof(unsigned short));
	starts_ = new Vector2D[heroSize_];
	heros_ = new Hero[heroSize_];
	for (unsigned i = 0; i < heroSize_; i++){
		in.read((char*)&starts_[i], sizeof(Vector2D));
	}

	//Room structure
	unsigned short size;
	in.read((char*)&size, sizeof(unsigned short));
	rooms_.resize(size);
	for (unsigned i = 0; i < size; i++){
		vector<Field*>& temp = rooms_[i];
		unsigned short fields;
		in.read((char*)&fields, sizeof(unsigned short));
		temp.resize(fields);
		for (unsigned j = 0; j < fields; j++){
			Vector2D v;
			in.read((char*)&v, sizeof(Vector2D));
			Field* f = &world_[v.y][v.x];
			temp[j] = f;
		}
	}
 
	//Doors
	in.read((char*)&size, sizeof(size));
	doors_.resize(size);
	for (unsigned i = 0; i < size; i++){
		Door* d = new Door();
		d->read(in);
		d->update();
		doors_[i] = d;
	}
 
	//Monsters
	in.read((char*)&size, sizeof(size));
	monsters_.resize(size);
	for(unsigned i=0; i<size; i++){
		monsterPos mp;
		in.read((char*)&mp, sizeof(mp));
		string monster = string(mp.monster);
		for(unsigned j=0; j<Templates::instance()->numMonsterTypes(); j++){
			if(Templates::instance()->getMonster(j).getBrev() == monster){
				Monster m = Templates::instance()->getMonster(j);
        Monster* ptr = new Monster();
        *ptr = m;
				addMonster(ptr, mp.pos, i);
				break;
			}
		}
	}

	//Furniture
	in.read((char*)&size, sizeof(size));
	furniture_.resize(size);
	for(unsigned i=0; i<size; i++){
		furniturePos fp;
		in.read((char*)&fp, sizeof(fp));
		string furniture = string(fp.furniture);
		
		bool b1, b2;
		Direction d;
		isupper(fp.furniture[0])? b1=true : b1 = false;
		isupper(fp.furniture[1])? b2=true : b2 = false;
		if(!b1 && !b2)
			d = TOP;
		else if (!b1 && b2)
			d = RIGHT;
		else if (b1 && !b2)
			d = BOTTOM;
		else 
			d = LEFT;
		
		furniture = toLower(furniture);
		//cerr<<"Furniture: "<<furniture<<endl;	
		for(unsigned j=0; j<furnitureTypes_.size(); j++){
			if(furnitureTypes_[j]->getBrev() == furniture){
				Furniture* f = new Furniture();
        *f = *furnitureTypes_[j];
				addFurniture(f, fp.pos, d, i);
				break;
			}
		}
	}

  //overlays
  in.read((char*)&size, sizeof(size));
	overlays_.resize(size);
	for(unsigned i=0; i<size; i++){
		overlayPos op;
		in.read((char*)&op, sizeof(op));
		string overlay = string(op.overlay);
		
		bool b1, b2;
		Direction d;
		isupper(op.overlay[0])? b1=true : b1 = false;
		isupper(op.overlay[1])? b2=true : b2 = false;
		if(!b1 && !b2)
			d = TOP;
		else if (!b1 && b2)
			d = RIGHT;
		else if (b1 && !b2)
			d = BOTTOM;
		else 
			d = LEFT;
		
		overlay = toLower(overlay);
		//cerr<<"Furniture: "<<furniture<<endl;
    
		Overlay* o = new Overlay();
    *o = *Templates::instance()->getOverlay(overlay);
    addOverlay(o, op.pos, d, i);
	}
  
  //scripts
  vector<scriptPos> scripts;
  in.read((char*)&size, sizeof(size));
  scripts.resize(size);
  for(unsigned i = 0; i<size; i++){
    scriptPos sp;
    in.read((char*)&sp, sizeof(sp));
    scripts[i] = sp;
  }
					
	in.close();
	//assert(_CrtCheckMemory());
  
	loaded_ = true;
  
#ifdef _CLIENT_
	//Collision update
	vWorld_ = new Vector3D*[976];
	updateCollisionVertices(cam.modelPos());
#endif

  //load level script
  scr.loadLevelScript(name);
  scr.processEvents(scripts);

	return true;
}

//Render world in 3D
void World::render(){
#ifdef _CLIENT_
	if (!isLoaded())
		return;
	glColor3f(1,1,1);
  
	//reset canSee_ and calculate new pvs
	for (unsigned i = 0; i < canSee_.size(); i++){
		canSee_[i]->tempVis.unset(2);
		canSee_[i]->tempVis.unset(3);
	}
	canSee_.clear();
	Direction dir = cam.getLookDirection();
	currently_visible(cam.modelPos(),dir);
  
	//print a nice debug message
	ostringstream fields;
	unsigned size = (unsigned)canSee_.size();
	fields << size << " of " << (width_*height_) << " Fields rendered";
  System::Engine::instance()->getFont()->setColor(0,1,0);
  System::Engine::instance()->getFont()->glPrint(20,680, fields.str().c_str(), 0, 0);

	//reset furniture
	for (unsigned i = 0; i < furniture_.size(); i++){
		furniture_[i]->reset();
	}
  
	//Render scene
	for (unsigned i = 0; i < canSee_.size(); i++){
		Field& curr = *canSee_[i];

		//render ground tiles
		glBindTexture(GL_TEXTURE_2D, tex.floorTex[curr.getId()]);
		glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(curr.texCoords[0].x, curr.texCoords[0].y);
			glVertex3f(curr.vertices[0].x, curr.vertices[0].y, curr.vertices[0].z);
			glTexCoord2f(curr.texCoords[1].x, curr.texCoords[1].y);
			glVertex3f(curr.vertices[1].x, curr.vertices[1].y, curr.vertices[1].z);
			glTexCoord2f(curr.texCoords[2].x, curr.texCoords[2].y);
			glVertex3f(curr.vertices[2].x, curr.vertices[2].y, curr.vertices[2].z);
			glTexCoord2f(curr.texCoords[3].x, curr.texCoords[3].y);
			glVertex3f(curr.vertices[3].x, curr.vertices[3].y, curr.vertices[3].z);
		glEnd();

		//render walls
		glBindTexture(GL_TEXTURE_2D, tex.wallTex[0]);
		short num = curr.doorbits.numSetBits()*3;
		short count = 0;
		for (unsigned k = 1; k < curr.numVertices/4; k++){
			if (count >= 3)
				count = 0;
			if(k >= curr.numVertices/4-num)
				count++;
			glBegin(GL_TRIANGLE_STRIP);
				glTexCoord2f(curr.texCoords[k*4+0].x, curr.texCoords[k*4+0].y);
				glVertex3f(curr.vertices[k*4+0].x,curr.vertices[k*4+0].y,curr.vertices[k*4+0].z);
				glTexCoord2f(curr.texCoords[k*4+1].x, curr.texCoords[k*4+1].y);
				glVertex3f(curr.vertices[k*4+1].x,curr.vertices[k*4+1].y,curr.vertices[k*4+1].z);
				glTexCoord2f(curr.texCoords[k*4+2].x, curr.texCoords[k*4+2].y);
				glVertex3f(curr.vertices[k*4+2].x,curr.vertices[k*4+2].y,curr.vertices[k*4+2].z);
				glTexCoord2f(curr.texCoords[k*4+3].x, curr.texCoords[k*4+3].y);
				glVertex3f(curr.vertices[k*4+3].x,curr.vertices[k*4+3].y,curr.vertices[k*4+3].z);
			glEnd();
		}
    
		//if there are objects on the field, render them, too
		if (curr.object){
			if (curr.object->getStatus()){
				curr.object->render();
				//furniture needs to be updated
				Furniture* f = dynamic_cast<Furniture*>(curr.object);
				if (f)
					f->update();
			}
		}
		if (curr.tryObject){
			if (curr.tryObject->getStatus())
				curr.tryObject->render();
		}

		//render doors
		if (curr.doorbits.getData() != '\0'){
			if (curr.doorbits.test(BOTTOM)){
				int idx = curr.dooridx[BOTTOM];
				doors_[idx]->render();
			}
			if (curr.doorbits.test(RIGHT)){
				int idx = curr.dooridx[RIGHT];
				doors_[idx]->render();
			}
			if (curr.doorbits.test(TOP)){
				int idx = curr.dooridx[TOP];
				doors_[idx]->render();
			}
			if (curr.doorbits.test(LEFT)){
				int idx = curr.dooridx[LEFT];
				doors_[idx]->render();
			}
		}
	}
  
	//render ceiling
	glBindTexture(GL_TEXTURE_2D, tex.wallTex[2]);
	glBegin(GL_QUADS);
		glTexCoord2f(width_,height_);
		glVertex3f(0, WALLHEIGHT, 0);
		glTexCoord2f(0,height_);
		glVertex3f(QUADSIZE*width_, WALLHEIGHT, 0);
		glTexCoord2f(0,0);
		glVertex3f(QUADSIZE*width_, WALLHEIGHT, QUADSIZE*height_);
		glTexCoord2f(width_,0);
		glVertex3f(0, WALLHEIGHT, QUADSIZE*height_);
	glEnd();
#endif
}

//Render 2D map
void World::render2D(bool vis){
#ifdef _CLIENT_
	if (!isLoaded())
		return;

	//Setup orthographic view
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, SCREENWIDTH, 0, SCREENHEIGHT,  -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	//reset furniture
	for (unsigned i = 0; i < furniture_.size(); i++){
		furniture_[i]->reset();
	}
 
	//reset overlay
	for (unsigned i = 0; i < overlays_.size(); i++){
		overlays_[i]->reset();
	}
  
	//how big may a single field be that all fit on the screen
	int xstep = SCREENWIDTH/width_;
	int ystep = SCREENHEIGHT/height_;
  
	glColor4f(1,1,1,1);
	glLineWidth(3);
	for (int j = 0; j < height_; j++){
		for (int i = 0; i < width_; i++){
			Field& curr = world_[j][i];
			// ground tiles
			// not visible?
			if (!curr.getStatus() && vis)
				glBindTexture(GL_TEXTURE_2D, tex.floorTex[0]);
			else
				glBindTexture(GL_TEXTURE_2D, tex.floorTex[curr.getId()]);
      
			if ((!curr.object && 
            (!curr.overlay || !curr.overlay->getStatus())
          ) 
          || (!curr.getStatus() && vis)){
				glBegin(GL_TRIANGLE_STRIP);
					glTexCoord2f(1.0f, 1.0f);
					glVertex2f(i*xstep+xstep, (height_-j)*ystep);
					glTexCoord2f(0.0f, 1.0f);
					glVertex2f(i*xstep, (height_-j)*ystep);
					glTexCoord2f(1.0f, 0.0f);
					glVertex2f(i*xstep+xstep, (height_-j)*ystep-ystep);
					glTexCoord2f(0.0f, 0.0f);
					glVertex2f(i*xstep, (height_-j)*ystep-ystep);
				glEnd();
			}
     
			
      glColor3f(1,1,1);
			
      if ((curr.getStatus() || !vis) && curr.overlay){
				curr.overlay->render2D();
			  curr.overlay->update();
      }
       
			//if there's an object, render it too.
			if ((curr.getStatus() || !vis) && curr.object){
				curr.object->render2D();
				Furniture* f = dynamic_cast<Furniture*>(curr.object);
				if (f)
					f->update();
			}

      
			glDisable(GL_TEXTURE_2D);
      
			//mark starting positions as long as the game did not start
			if (game.getState() == PREPARE){
				glColor3f(1,0,0);
				for (int k = 0; k < heroSize_; k++){
					if (Vector2D(i,j) == starts_[k]){
						glBegin(GL_LINES);
							glVertex2f(i*xstep+3, (height_-j)*ystep-ystep+3);
							glVertex2f(i*xstep+xstep-3, (height_-j)*ystep-3);
							glVertex2f(i*xstep+3, (height_-j)*ystep-2);
							glVertex2f(i*xstep+xstep-3, (height_-j)*ystep-ystep+3);
						glEnd();
					}
				}
			}
      
			//draw walls and doors as colored lines
			//glDisable(GL_BLEND);
			glBegin(GL_LINES);
				glColor3f(1,1,1);
				if (isWall(i,j,TOP, vis)){
					glVertex2f(i*xstep, (height_-j)*ystep);
					glVertex2f(i*xstep+xstep, (height_-j)*ystep);
				}
				if (isWall(i,j,RIGHT, vis)){
					glVertex2f(i*xstep+xstep, (height_-j)*ystep);
					glVertex2f(i*xstep+xstep, (height_-j)*ystep-ystep);
				}
				if (isWall(i,j,BOTTOM, vis)){
					glVertex2f(i*xstep, (height_-j)*ystep-ystep);
					glVertex2f(i*xstep+xstep, (height_-j)*ystep-ystep);
				}
				if (isWall(i,j,LEFT, vis)){
					glVertex2f(i*xstep, (height_-j)*ystep-ystep);
					glVertex2f(i*xstep, (height_-j)*ystep);
				}
				if (isDoor(i,j,TOP, vis)){
					Door* doo = getDoor(Vector2D(i,j),TOP);
					if (doo->getId() > 0)
						glColor3f(1,1,1);
					else
        		glColor3f(0.8,0.12,0.12);
					if(doo->getStatus()){
						glVertex2f(i*xstep, (height_-j)*ystep);
						glVertex2f(i*xstep+xstep, (height_-j)*ystep);
					}
				}
				if (isDoor(i,j,RIGHT, vis)){
					Door* doo = getDoor(Vector2D(i,j),RIGHT);
					if (doo->getId() > 0)
						glColor3f(1,1,1);
					else
        		glColor3f(0.8,0.12,0.12);
					if(doo->getStatus()){
						glVertex2f(i*xstep+xstep, (height_-j)*ystep);
						glVertex2f(i*xstep+xstep, (height_-j)*ystep-ystep);
					}
				}
				if (isDoor(i,j,BOTTOM, vis)){
					Door* doo = getDoor(Vector2D(i,j),BOTTOM);
					if (doo->getId() > 0)
						glColor3f(1,1,1);
					else
        		glColor3f(0.8,0.12,0.12);
					if(doo->getStatus()){
						glVertex2f(i*xstep, (height_-j)*ystep-ystep);
						glVertex2f(i*xstep+xstep, (height_-j)*ystep-ystep);
					}
				}
				if (isDoor(i,j,LEFT, vis)){
					Door* doo = getDoor(Vector2D(i,j),LEFT);
					if (doo->getId() > 0)
						glColor3f(1,1,1);
					else
        		glColor3f(0.8,0.12,0.12);
					if(doo->getStatus()){
						glVertex2f(i*xstep, (height_-j)*ystep-ystep);
						glVertex2f(i*xstep, (height_-j)*ystep);
					}
				}
	
			glEnd();
		
      glColor3f(1.0,1.0,1.0);
      //glEnable(GL_BLEND);
			
      glEnable(GL_TEXTURE_2D);
		}
	}

	//Render orientation line (the direction in which the camera looks)
	glDisable(GL_TEXTURE_2D);
	//glDisable(GL_BLEND);
	glColor3f(1,1,0);
	Vector3D view = (cam.view() - cam.position()).normalized();
	Vector2D pos = cam.modelPos();
	glBegin(GL_LINES);
		glVertex2f((pos.x+0.5)*xstep, (height_-pos.y-0.5)*ystep);
		glVertex2f((pos.x+0.5)*xstep + 0.5*xstep*view.x, (height_-pos.y-0.5)*ystep - 0.5*ystep*view.z);
	glEnd();
	glColor3f(1,1,1);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
  
	//in debug compilation, visualize all fields that will be rendered in 3D on the 2D map.   
#ifdef _DEBUG_
	//what is visible in 3D?
	currently_visible(cam.modelPos(), cam.getLookDirection());
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glColor3f(0,1,0);
	for (unsigned k = 0; k < canSee_.size(); k++){
		int i = canSee_[k]->getPosition().x;
		int j = canSee_[k]->getPosition().y;
		glBegin(GL_LINES);
			glVertex2f(i*xstep+3, (height_-j)*ystep-ystep+3);
			glVertex2f(i*xstep+xstep-3, (height_-j)*ystep-3);
			glVertex2f(i*xstep+3, (height_-j)*ystep-2);
			glVertex2f(i*xstep+xstep-3, (height_-j)*ystep-ystep+3);
		glEnd();
	}
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
#endif
  
	//Restore original
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();
#endif
}

//update vertices for collision detection
void World::updateCollisionVertices(Vector2D modelPos){
#ifdef _CLIENT_
	if (!isLoaded()){
		numberOfVerts_ = 0;
		return;
	}
  // Create a counter for the index of the vWorld_[] array
  int index = 0;
	
  //Check the walls on the field of the position of the camera
  int j = (int)modelPos.y;
  int i = (int)modelPos.x;
  if (i < 0 || i >= width_)
    i = 0;
  if (j < 0 || j >= height_)
    j = 0;
  for (unsigned k = 0; k < world_[j][i].numVertices/4; k++){
    if (k > 0 && !wallCollision_)
	    break;
    vWorld_[index++] = &world_[j][i].vertices[k*4+0];
    vWorld_[index++] = &world_[j][i].vertices[k*4+1];
    vWorld_[index++] = &world_[j][i].vertices[k*4+2];
    vWorld_[index++] = &world_[j][i].vertices[k*4+2];
    vWorld_[index++] = &world_[j][i].vertices[k*4+1];
    vWorld_[index++] = &world_[j][i].vertices[k*4+3];
  }

  //Check door collisions
  Door* doo;
  doo = getDoor(Vector2D(i,j), TOP);
  if (doo){
    if (doo->getStatus()){
      vWorld_[index++] = &doo->getColVerts()[4];
      vWorld_[index++] = &doo->getColVerts()[5];
      vWorld_[index++] = &doo->getColVerts()[6];
      vWorld_[index++] = &doo->getColVerts()[6];
      vWorld_[index++] = &doo->getColVerts()[5];
      vWorld_[index++] = &doo->getColVerts()[7];
    }
  }
  doo = getDoor(Vector2D(i,j), RIGHT);
  if (doo){
    if (doo->getStatus()){
      vWorld_[index++] = &doo->getColVerts()[0];
      vWorld_[index++] = &doo->getColVerts()[1];
      vWorld_[index++] = &doo->getColVerts()[2];
      vWorld_[index++] = &doo->getColVerts()[2];
      vWorld_[index++] = &doo->getColVerts()[1];
      vWorld_[index++] = &doo->getColVerts()[3];
    }
  }
  doo = getDoor(Vector2D(i,j), BOTTOM);
  if (doo){
    if (doo->getStatus()){
      vWorld_[index++] = &doo->getColVerts()[0];
      vWorld_[index++] = &doo->getColVerts()[1];
      vWorld_[index++] = &doo->getColVerts()[2];
      vWorld_[index++] = &doo->getColVerts()[2];
      vWorld_[index++] = &doo->getColVerts()[1];
      vWorld_[index++] = &doo->getColVerts()[3];
    }
  }
  doo = getDoor(Vector2D(i,j), LEFT);
  if (doo){
    if (doo->getStatus()){
      vWorld_[index++] = &doo->getColVerts()[4];
      vWorld_[index++] = &doo->getColVerts()[5];
      vWorld_[index++] = &doo->getColVerts()[6];
      vWorld_[index++] = &doo->getColVerts()[6];
      vWorld_[index++] = &doo->getColVerts()[5];
      vWorld_[index++] = &doo->getColVerts()[7];
    }
  }

  //set invisible box around the field, if no more moves are left
  //if (game.getMoves() <= 0){
    moveBox_[0] = world_[j][i].vertices[0];
    moveBox_[1] = world_[j][i].vertices[1];
    moveBox_[2] = world_[j][i].vertices[2];
    moveBox_[3] = world_[j][i].vertices[3];
    moveBox_[0].y = WALL_HEIGHT;
    moveBox_[1].y = WALL_HEIGHT;
    moveBox_[2].y = WALL_HEIGHT;
    moveBox_[3].y = WALL_HEIGHT;

	Creature* c = plyr.getCreature();
	if (!c){
		numberOfVerts_ = index;
		return;
	}
	Vector2D p = c->getPosition();
	bool selfHero = dynamic_cast<Hero*>(world_[p.y][p.x].object) == NULL ? false : true;
	
	//TOP
	bool objectHero = true;
	bool objectMonster = true;
  bool objectFurni = false;
  bool overlayNotWalkable = false;
  bool outside = false;
  if (i >= width_ || i < 0 || j-1 >= height_ || j-1 < 0){
    outside = true;
  }
	GameObject *o = wrld.getObject(Vector2D(i, j-1));
	if (o){
		objectHero = dynamic_cast<Hero*>(o) == NULL ? false : true;
		objectMonster = dynamic_cast<Monster*>(o) == NULL ? false : true;
    objectFurni = dynamic_cast<Furniture*>(o) == NULL ? false : true;
	}
  if (!outside){
    Field* f = &wrld.getField(Vector2D(i, j-1));
    if (f->overlay && f->overlay->getStatus() && !f->overlay->isWalkable())
      overlayNotWalkable = true;
  }
	//cerr << selfHero << " " << objectHero << " " << objectMonster << "\n";
	
	if (game.getMoves() <= 0 || ((selfHero && !objectHero) && respectClasses_) || ((!selfHero && !objectMonster) && respectClasses_) || objectFurni || overlayNotWalkable){
		vWorld_[index++] = &moveBox_[0];
		vWorld_[index++] = &moveBox_[1];
		vWorld_[index++] = &world_[j][i].vertices[0];
		vWorld_[index++] = &world_[j][i].vertices[0];
		vWorld_[index++] = &moveBox_[1];
		vWorld_[index++] = &world_[j][i].vertices[1];
	}
    
	//LEFT
	objectHero = true;
	objectMonster = true;
  objectFurni = false;
  overlayNotWalkable = false;
  outside = false;
  if (i-1 >= width_ || i-1 < 0 || j >= height_ || j < 0){
    outside = true;
  }
	o = wrld.getObject(Vector2D(i-1, j));
	if (o){
		objectHero = dynamic_cast<Hero*>(o) == NULL ? false : true;
		objectMonster = dynamic_cast<Monster*>(o) == NULL ? false : true;
    objectFurni = dynamic_cast<Furniture*>(o) == NULL ? false : true;
	}
  if (!outside){
    Field* f = &wrld.getField(Vector2D(i-1, j));
    if (f->overlay && f->overlay->getStatus() && !f->overlay->isWalkable())
      overlayNotWalkable = true;
  }
	if (game.getMoves() <= 0 || ((selfHero && !objectHero) && respectClasses_) || ((!selfHero && !objectMonster) && respectClasses_) || objectFurni || overlayNotWalkable){
		vWorld_[index++] = &moveBox_[1];
		vWorld_[index++] = &moveBox_[3];
		vWorld_[index++] = &world_[j][i].vertices[1];
		vWorld_[index++] = &world_[j][i].vertices[1];
		vWorld_[index++] = &moveBox_[3];
		vWorld_[index++] = &world_[j][i].vertices[3];
	}

	//BOTTOM
	objectHero = true;
	objectMonster = true;
  objectFurni = false;
  overlayNotWalkable = false;
  outside = false;
  if (i >= width_ || i < 0 || j+1 >= height_ || j+1 < 0){
    outside = true;
  }
	o = wrld.getObject(Vector2D(i, j+1));
	if (o){
		objectHero = dynamic_cast<Hero*>(o) == NULL ? false : true;
		objectMonster = dynamic_cast<Monster*>(o) == NULL ? false : true;
    objectFurni = dynamic_cast<Furniture*>(o) == NULL ? false : true;
	}
  if (!outside){
    Field* f = &wrld.getField(Vector2D(i, j+1));
    if (f->overlay && f->overlay->getStatus() && !f->overlay->isWalkable())
      overlayNotWalkable = true;
  }
	if (game.getMoves() <= 0 || ((selfHero && !objectHero) && respectClasses_) || ((!selfHero && !objectMonster) && respectClasses_) || objectFurni || overlayNotWalkable){
		vWorld_[index++] = &moveBox_[3];
		vWorld_[index++] = &moveBox_[2];
		vWorld_[index++] = &world_[j][i].vertices[3];
		vWorld_[index++] = &world_[j][i].vertices[3];
		vWorld_[index++] = &moveBox_[2];
		vWorld_[index++] = &world_[j][i].vertices[2];
	}

	//RIGHT
	objectHero = true;
	objectMonster = true;
  objectFurni = false;
  overlayNotWalkable = false;
  outside = false;
  if (i+1 >= width_ || i+1 < 0 || j >= height_ || j < 0){
    outside = true;
  }
	o = wrld.getObject(Vector2D(i+1, j));
	if (o){
		objectHero = dynamic_cast<Hero*>(o) == NULL ? false : true;
		objectMonster = dynamic_cast<Monster*>(o) == NULL ? false : true;
    objectFurni = dynamic_cast<Furniture*>(o) == NULL ? false : true;
	}
  if (!outside){
    Field* f = &wrld.getField(Vector2D(i+1, j));
    if (f->overlay && f->overlay->getStatus() && !f->overlay->isWalkable())
      overlayNotWalkable = true;
  }
	if (game.getMoves() <= 0 || ((selfHero && !objectHero) && respectClasses_) || ((!selfHero && !objectMonster) && respectClasses_) || objectFurni || overlayNotWalkable){
		vWorld_[index++] = &moveBox_[2];
		vWorld_[index++] = &moveBox_[0];
		vWorld_[index++] = &world_[j][i].vertices[2];
		vWorld_[index++] = &world_[j][i].vertices[2];
		vWorld_[index++] = &moveBox_[0];
		vWorld_[index++] = &world_[j][i].vertices[0];
	}
  //}

  //Check walls of neighbouring fields
  int x = i+1;
  int y = j;
  if (x < 0 || x >= width_)
    x = 0;
  if (y < 0 || y >= height_)
    y = 0;
  for (unsigned k = 0; k < world_[y][x].numVertices/4; k++){
    vWorld_[index++] = &world_[y][x].vertices[k*4+0];
    vWorld_[index++] = &world_[y][x].vertices[k*4+1];
    vWorld_[index++] = &world_[y][x].vertices[k*4+2];
    vWorld_[index++] = &world_[y][x].vertices[k*4+2];
    vWorld_[index++] = &world_[y][x].vertices[k*4+1];
    vWorld_[index++] = &world_[y][x].vertices[k*4+3];
  }

  x = i-1;
  y = j;
  if (x < 0 || x >= width_)
    x = 0;
  if (y < 0 || y >= height_)
    y = 0;
  for (unsigned k = 0; k < world_[y][x].numVertices/4; k++){
    vWorld_[index++] = &world_[y][x].vertices[k*4+0];
    vWorld_[index++] = &world_[y][x].vertices[k*4+1];
    vWorld_[index++] = &world_[y][x].vertices[k*4+2];
    vWorld_[index++] = &world_[y][x].vertices[k*4+2];
    vWorld_[index++] = &world_[y][x].vertices[k*4+1];
    vWorld_[index++] = &world_[y][x].vertices[k*4+3];
  }

  x = i;
  y = j+1;
  if (x < 0 || x >= width_)
    x = 0;
  if (y < 0 || y >= height_)
    y = 0;
  for (unsigned k = 0; k < world_[y][x].numVertices/4; k++){
    vWorld_[index++] = &world_[y][x].vertices[k*4+0];
    vWorld_[index++] = &world_[y][x].vertices[k*4+1];
    vWorld_[index++] = &world_[y][x].vertices[k*4+2];
    vWorld_[index++] = &world_[y][x].vertices[k*4+2];
    vWorld_[index++] = &world_[y][x].vertices[k*4+1];
    vWorld_[index++] = &world_[y][x].vertices[k*4+3];
  }

  x = i;
  y = j-1;
  if (x < 0 || x >= width_)
    x = 0;
  if (y < 0 || y >= height_)
    y = 0;
  for (unsigned k = 0; k < world_[y][x].numVertices/4; k++){
    vWorld_[index++] = &world_[y][x].vertices[k*4+0];
    vWorld_[index++] = &world_[y][x].vertices[k*4+1];
    vWorld_[index++] = &world_[y][x].vertices[k*4+2];
    vWorld_[index++] = &world_[y][x].vertices[k*4+2];
    vWorld_[index++] = &world_[y][x].vertices[k*4+1];
    vWorld_[index++] = &world_[y][x].vertices[k*4+3];
  }

  //this is finally the number of vertices for collision detection
  numberOfVerts_ = index;
  assert(index < 976);
#endif
}

//set an GameObject on the map
bool World::setObject(GameObject* o, const Vector2D& pos, bool interpolate){
  Vector2D oldPos = o->getPosition();
  Field *oldField = &world_[oldPos.y][oldPos.x];
  Field *newField = &world_[pos.y][pos.x];
  
  //is the field already occupied by someone else?
  GameObject* occupier = newField->object;
  if (occupier){
    //does it not matter, which type there is
    if (!respectClasses_){
      //Is the occupier a creature
      Creature* creat = dynamic_cast<Creature*>(occupier);
      if (creat){
        Creature* self = dynamic_cast<Creature*>(o);
        if (self){
          oldField->object = NULL;
          o->setPosition(pos,interpolate);
          newField->tryObject = newField->object;
          newField->object = o;
          if (oldField->tryObject == NULL){
            self->setBackPointer(oldField);
          }
          else{
            oldField->object = oldField->tryObject;
            oldField->tryObject = NULL;
          }
#ifdef _CLIENT_
          if (realToModelPos(cam.position()) != pos){
            if (plyr.getActCreature() == self->getName() && plyr.yourTurn()){
              Vector3D p = modelToRealPos(pos);
              cam.positionCamera(p, Vector3D(p.x+1,p.y,p.z), Vector3D(0,1,0));
              updateCollisionVertices(pos);
            }
          }
#endif
          return true;
        }
        return false;
      }
      return false;
    }
    
    //Is the occupier a hero?
    Hero* hero = dynamic_cast<Hero*>(occupier);
    if (hero){
      //Am I myself a hero?
      Hero* self = dynamic_cast<Hero*>(o);
      if (self){
        //two heros are on the same field, so trying to move
	      oldField->object = NULL;
	      o->setPosition(pos,interpolate);
	      //save the old object as the tryObject
	      newField->tryObject = newField->object;
	      newField->object = o;
	      //where must the character be moved, if move try fails
	      if (oldField->tryObject == NULL){
	        self->setBackPointer(oldField);
	      }
	      else{
	        oldField->object = oldField->tryObject;
	        oldField->tryObject = NULL;
	      }
#ifdef _CLIENT_
	      //update camera, if neccessary
	      if (realToModelPos(cam.position()) != pos){
	        if (plyr.getActCreature() == self->getName() && plyr.yourTurn()){
              Vector3D p = modelToRealPos(pos);
              cam.positionCamera(p, Vector3D(p.x+1,p.y,p.z), Vector3D(0,1,0));
	            updateCollisionVertices(pos);
	        }
	      }
#endif
	      return true;
      }
      return false;
    }
    //Is the occupier a monster?
    Monster* monsty = dynamic_cast<Monster*>(occupier);
    if (monsty){
      //Am I myself a monster?
      Monster* self = dynamic_cast<Monster*>(o);
      if (self){
        //two monsters are on the same field, so trying to move
	      oldField->object = NULL;
	      o->setPosition(pos,interpolate);
	      //save the old object as the tryObject
	      newField->tryObject = newField->object;
	      newField->object = o;
	      //where must the character be moved, if move try fails
	      if (oldField->tryObject == NULL){
	        self->setBackPointer(oldField);
	      }
	      else{
	        oldField->object = oldField->tryObject;
	        oldField->tryObject = NULL;
	      }
#ifdef _CLIENT_
	      //update camera, if neccessary
	      if (realToModelPos(cam.position()) != pos){
	        if (plyr.getActCreature() == self->getName() && plyr.yourTurn()){
              Vector3D p = modelToRealPos(pos);
              cam.positionCamera(p, Vector3D(p.x+1,p.y,p.z), Vector3D(0,1,0));
	            updateCollisionVertices(pos);
	        }
	      }
#endif
	      return true;
      }
      return false;
    }
  }
  //standard case (move to a free field)
  oldField->object = NULL;
  o->setPosition(pos, interpolate);
  newField->object = o;
  //was it only trying to move the last time
  Creature* self = dynamic_cast<Creature*>(o);
  if (self){
    //only heros update the visibility
    Hero* h = dynamic_cast<Hero*>(self);
    if (h)
      update_visibility(h->getPosition());
    self->setBackPointer(NULL);
    if (oldField->tryObject){
      //set original back
      oldField->object = oldField->tryObject;
      oldField->tryObject = NULL;
    }
    //is a trap on the new field ==> set backpointer
    if (newField->script && islower(newField->script->script[0])){
      self->setBackPointer(oldField);
    }
#ifdef _CLIENT_
    //update camera
    if (realToModelPos(cam.position()) != pos){
      if (plyr.getActCreature() == self->getName() && plyr.yourTurn()){
        Vector3D p = modelToRealPos(pos);
        cam.positionCamera(p, Vector3D(p.x+1,p.y,p.z), Vector3D(0,1,0));
	      updateCollisionVertices(pos);
      }
    }
#endif
  }
  return true;
}

//set object anchor
void World::setObjectAnchor(GameObject* o, const Vector2D& pos){
	world_[pos.y][pos.x].object = o;
}

//set object anchor
void World::setObjectAnchor(Overlay* o, const Vector2D& pos){
	world_[pos.y][pos.x].overlay = o;
}

//remove an GameObject from the map
void World::removeObject(const Vector2D& pos){
  world_[pos.y][pos.x].object = NULL;
}

//add a hero to the map
void World::addHero(const Hero& heroe, const short posIdx){
  starts_[posIdx] = Vector2D(-1,-1);
  heros_[posIdx] = heroe;
  Vector2D pos = heroe.getPosition();
  wrld.setObject(&heros_[posIdx], pos);
#ifdef _CLIENT_
  if (heroe.getPlayer() == plyr.getName()){
    Vector3D p = modelToRealPos(pos);
    cam.positionCamera(p, Vector3D(p.x+1,p.y,p.z), Vector3D(0,1,0));
    updateCollisionVertices(pos);
  }
#endif
}

//add a monster to the map
void World::addMonster(Monster* monsta, const Vector2D& pos, int vecPos){
	monsta->setPosition(pos);
	short mid = monsta->getId();
	monsterCounter_[mid]++;
	string name = monsta->getType() + toStr(monsterCounter_[mid]);
	monsta->setName(name);
  monsters_[vecPos] = monsta;
	
  wrld.setObject(monsters_[vecPos], pos, false);
#ifdef _CLIENT_
  if (monsta->getPlayer() == plyr.getName()){
    Vector3D p = modelToRealPos(pos);
    cam.positionCamera(p, Vector3D(p.x+1,p.y,p.z), Vector3D(0,1,0));
    updateCollisionVertices(pos);
  }
#endif
}

//add a monster to the map
void World::addMonster(Monster* monsta, const Vector2D& pos){
	monsta->setPosition(pos);
	short mid = monsta->getId();
	monsterCounter_[mid]++;
	string name = monsta->getType() + toStr(monsterCounter_[mid]);
	monsta->setName(name);
  monsters_.push_back(monsta);
	
  wrld.setObject(monsters_.back(), pos, false);
}

//add furniture to the map
void World::addFurniture(Furniture* furni, const Vector2D& pos, Direction d, int vecPos){
	furni->setPosition(pos);
	furni->setOrientation(d);
	furniture_[vecPos] = furni;

	switch (d){
	case TOP:
		for (int j = 0; j < furni->getHeight(); j++){
			for (int i = 0; i < furni->getWidth(); i++){
				Vector2D pos2 = Vector2D(pos.x+i, pos.y+j);
				wrld.setObjectAnchor(furniture_[vecPos], pos2);
			}
		}
		break;
	case RIGHT:
		for (int j = 0; j < furni->getHeight(); j++){
			for (int i = 0; i < furni->getWidth(); i++){
				Vector2D pos2 = Vector2D(pos.x-j, pos.y+i);
				wrld.setObjectAnchor(furniture_[vecPos], pos2);
			}
		}
		break;
	case BOTTOM:
		for (int j = 0; j < furni->getHeight(); j++){
			for (int i = 0; i < furni->getWidth(); i++){
				Vector2D pos2 = Vector2D(pos.x-i, pos.y-j);
				wrld.setObjectAnchor(furniture_[vecPos], pos2);
			}
		}
		break;
	case LEFT:
		for (int j = 0; j < furni->getHeight(); j++){
			for (int i = 0; i < furni->getWidth(); i++){
				Vector2D pos2 = Vector2D(pos.x+j, pos.y-i);
				wrld.setObjectAnchor(furniture_[vecPos], pos2);
			}
		}
		break;
	}
	//wrld.setObject(&furniture_[vecPos], pos, false);
}

//add overlay to the map
void World::addOverlay(Overlay* over, const Vector2D& pos, Direction d, int vecPos){
  over->setPosition(pos);
  over->setOrientation(d);
  //TODO this is a temporary hack as long as traps are not fully implemented
  //only stairs and traps known; stairs must be visible
  if (over->getId() == 0)
    over->setActive();
  
  overlays_[vecPos] = over;

  switch (d){
  case TOP:
    for (int j = 0; j < over->getHeight(); j++){
      for (int i = 0; i < over->getWidth(); i++){
        Vector2D pos2 = Vector2D(pos.x+i, pos.y+j);
        wrld.setObjectAnchor(overlays_[vecPos], pos2);
      }
    }
    break;
  case RIGHT:
    for (int j = 0; j < over->getHeight(); j++){
      for (int i = 0; i < over->getWidth(); i++){
        Vector2D pos2 = Vector2D(pos.x-j, pos.y+i);
        wrld.setObjectAnchor(overlays_[vecPos], pos2);
      }
    }
    break;
  case BOTTOM:
    for (int j = 0; j < over->getHeight(); j++){
      for (int i = 0; i < over->getWidth(); i++){
        Vector2D pos2 = Vector2D(pos.x-i, pos.y-j);
        wrld.setObjectAnchor(overlays_[vecPos], pos2);
      }
    }
    break;
  case LEFT:
    for (int j = 0; j < over->getHeight(); j++){
      for (int i = 0; i < over->getWidth(); i++){
        Vector2D pos2 = Vector2D(pos.x+j, pos.y-i);
        wrld.setObjectAnchor(overlays_[vecPos], pos2);
      }
    }
    break;
  }
  //wrld.setObject(&furniture_[vecPos], pos, false);
  
}

//read in monster and furniture data
void World::init(){
	//monsters
	monsterCounter_.resize(Templates::instance()->numMonsterTypes());
	//for (unsigned i = 0; i < monsterTypes_.size(); i++){
	//  monsterCounter_[i] = 0;
	//}
	memset(&monsterCounter_[0], 0, sizeof(unsigned short) * Templates::instance()->numMonsterTypes());
	
	//furniture
  string temp;
	string file = path + "../heroquest/data/furniture.dat";
	ifstream in2(file.c_str());
	while(in2 >> temp){
		Furniture* f = new Furniture();
	//	f.setType(temp);
		in2>>temp; in2>>temp; in2>>temp;
		f->setId(toInt(temp));
		in2>>temp; in2>>temp;
		f->setBrev(temp);
		in2>>temp; in2>>temp;
		f->setWidth(toInt(temp));
		in2>>temp; in2>>temp;
		f->setHeight(toInt(temp));
		furnitureTypes_.push_back(f);
	}
	in2.close();

}

//updates 2D map visibility
void World::update_visibility(const Vector2D& pos, bool subsequent, Vector2D oldPos){
  if (!isLoaded())
    return;

  short roomId = world_[pos.y][pos.x].getId();

  //on the floor, use a kind of seed fill
  if (roomId == 0 || subsequent){
    // a room is reached
    if (roomId != 0)
      return;
    //init tempVis for seed fill
    if (!subsequent){
      for (unsigned i = 0; i < rooms_[0].size(); i++){
        Field* f = rooms_[0][i];
        f->tempVis.unset(1);
      }
      oldPos = pos;
    }
    //we can see this field now
    world_[pos.y][pos.x].setActive();
    world_[pos.y][pos.x].tempVis.set(1);
    //make monsters movavle
    if (world_[pos.y][pos.x].object){
      Monster* m = dynamic_cast<Monster*>(world_[pos.y][pos.x].object);
      if (m){
        m->canMove();
      }
    }

    //seed-fill with 8-connectivity
    //does not continue if the new field was already reached or can't be seen.
    Vector2D next(pos.x+1, pos.y);
    if (!isTempVis(next,1) && isVisible(oldPos, next))
      update_visibility(next, true, oldPos);
    next = Vector2D(pos.x-1, pos.y);
    if (!isTempVis(next,1) && isVisible(oldPos, next))
      update_visibility(next, true, oldPos);
    next = Vector2D(pos.x, pos.y+1);
    if (!isTempVis(next,1) && isVisible(oldPos, next))
      update_visibility(next, true, oldPos);
    next = Vector2D(pos.x, pos.y-1);
    if (!isTempVis(next,1) && isVisible(oldPos, next))
      update_visibility(next, true, oldPos);
    next = Vector2D(pos.x+1, pos.y+1);
    if (!isTempVis(next,1) && isVisible(oldPos, next))
      update_visibility(next, true, oldPos);
    next = Vector2D(pos.x+1, pos.y-1);
    if (!isTempVis(next,1) && isVisible(oldPos, next))
      update_visibility(next, true, oldPos);
    next = Vector2D(pos.x-1, pos.y+1);
    if (!isTempVis(next,1) && isVisible(oldPos, next))
      update_visibility(next, true, oldPos);
    next = Vector2D(pos.x-1, pos.y-1);
    if (!isTempVis(next,1) && isVisible(oldPos, next))
      update_visibility(next, true, oldPos);
  }
  //in a room, everything is visible says the HeroQuest rule book, 
  //so it's an easy job in this case
  else{
    vector<Field*>& room = rooms_[roomId];
    for(unsigned i = 0; i < room.size(); i++){
      room[i]->setActive();
      //make monsters movavle
      if (room[i]->object){
        Monster* m = dynamic_cast<Monster*>(room[i]->object);
        if (m){
          m->canMove();
        }
      }
      
    }
  }
  
}

//updates 2D map visibility
vector<Field*> World::getVisibleFields(const Vector2D& pos, bool subsequent, Vector2D oldPos){
  if (!isLoaded())
    return visibleFields_;

  short roomId = world_[pos.y][pos.x].getId();

    //init tempVis for seed fill
    if (!subsequent){
      for (unsigned i = 0; i < rooms_[0].size(); i++){
        Field* f = rooms_[0][i];
        f->tempVis.unset(1);
      }
      visibleFields_.clear();
      oldPos = pos;
    }
    
  //on the floor, use a kind of seed fill
  if (roomId == 0 || subsequent){
    // a room is reached
    if (roomId != 0)
      return visibleFields_;
    //we can see this field
    visibleFields_.push_back(&world_[pos.y][pos.x]);
    world_[pos.y][pos.x].tempVis.set(1);

    //seed-fill with 8-connectivity
    //does not continue if the new field was already reached or can't be seen.
    Vector2D next(pos.x+1, pos.y);
    if (!isTempVis(next,1) && isVisible(oldPos, next))
      getVisibleFields(next, true, oldPos);
    next = Vector2D(pos.x-1, pos.y);
    if (!isTempVis(next,1) && isVisible(oldPos, next))
      getVisibleFields(next, true, oldPos);
    next = Vector2D(pos.x, pos.y+1);
    if (!isTempVis(next,1) && isVisible(oldPos, next))
      getVisibleFields(next, true, oldPos);
    next = Vector2D(pos.x, pos.y-1);
    if (!isTempVis(next,1) && isVisible(oldPos, next))
      getVisibleFields(next, true, oldPos);
    next = Vector2D(pos.x+1, pos.y+1);
    if (!isTempVis(next,1) && isVisible(oldPos, next))
      getVisibleFields(next, true, oldPos);
    next = Vector2D(pos.x+1, pos.y-1);
    if (!isTempVis(next,1) && isVisible(oldPos, next))
      getVisibleFields(next, true, oldPos);
    next = Vector2D(pos.x-1, pos.y+1);
    if (!isTempVis(next,1) && isVisible(oldPos, next))
      getVisibleFields(next, true, oldPos);
    next = Vector2D(pos.x-1, pos.y-1);
    if (!isTempVis(next,1) && isVisible(oldPos, next))
      getVisibleFields(next, true, oldPos);
    return visibleFields_;
  }
  //in a room, everything is visible says the HeroQuest rule book, 
  //so it's an easy job in this case
  else{
    vector<Field*>& room = rooms_[roomId];
    for(unsigned i = 0; i < room.size(); i++){
      visibleFields_.push_back(room[i]);
    }
    return visibleFields_;
  }
  
}

//approximates a potential visible set
void World::currently_visible(const Vector2D& pos, Direction look, short depth, short leftoffset, short rightoffset, short same, short idx){
 
#ifdef _CLIENT_
  //some criteria to stop calculation:
  //world not loaded?
  if (!isLoaded())
    return;
  //invalid field?
  if (pos.x < 0 || pos.y < 0 || pos.x >= width_ || pos.y >= height_)
    return;
  //can't be seen for sure?
  if (depth <= 0 || leftoffset <= 0  || rightoffset <= 0 || same >= 3)
    return;
  //already reached?
  if(world_[pos.y][pos.x].tempVis.test(idx))
    return;
  
  //the current field is not yet part of the pvs?
  if (!world_[pos.y][pos.x].tempVis.test(2) && !world_[pos.y][pos.x].tempVis.test(3) &&
		  !world_[pos.y][pos.x].tempVis.test(4) && !world_[pos.y][pos.x].tempVis.test(5) &&
		  !world_[pos.y][pos.x].tempVis.test(6) && !world_[pos.y][pos.x].tempVis.test(7))
    canSee_.push_back(&world_[pos.y][pos.x]);
  world_[pos.y][pos.x].tempVis.set(idx);
 
  //it's not easy to see what is going on in the next lines, but it's a heuristic approach
  //to determine where the algorithm has to spread further. If no obstacles are there, it
  //spreads as follows (assuming in this exmple the looking diretion to be BOTTOM)
  //             ooxoo
  //           ooooooooo
  //         ooooooooooooo
  //              ...
  //The same paramter is a regularizer that allows premature breaks. If no obstacles are there,
  //it has no important function. leftoffset and rightoffset generate this funny triangle-like-shape. 
  //The other parameters are self-explanatory.
  Vector2D next;
  if (look == BOTTOM){
    if (leftoffset > 1){
      next = Vector2D(pos.x-2, pos.y);
      if (!isWall(pos.x-1,pos.y,LEFT,false) && !isWall(pos.x,pos.y,LEFT,false)){
          currently_visible(next, look, depth, leftoffset-1, rightoffset+1, same+2, 3);
      }
    }
    next = Vector2D(pos.x-1, pos.y);
    if (!isWall(pos.x,pos.y,LEFT,false)){
        currently_visible(next, look, depth, leftoffset-1, rightoffset+1, same+1, 4);
    }
    next = Vector2D(pos.x, pos.y+1);
    if (!isWall(pos.x,pos.y,BOTTOM,false))
      currently_visible(next, look, depth-1, leftoffset+2, rightoffset+2);
    if (rightoffset > 1){
      next = Vector2D(pos.x+2, pos.y);
      if (!isWall(pos.x+1,pos.y,RIGHT,false) && !isWall(pos.x,pos.y,RIGHT,false)){
          currently_visible(next, look, depth, leftoffset+1, rightoffset-1, same+2, 5);
      }
    }
    next = Vector2D(pos.x+1, pos.y);
    if (!isWall(pos.x,pos.y,RIGHT,false)){
        currently_visible(next, look, depth, leftoffset+1, rightoffset-1, same+1,6);
    }
  }
  
  else if (look == RIGHT){
    if (rightoffset > 1){
      next = Vector2D(pos.x, pos.y+2);
      if (!isWall(pos.x,pos.y+1,BOTTOM,false) && !isWall(pos.x,pos.y,BOTTOM,false)){
	  currently_visible(next, look, depth, leftoffset+1, rightoffset-1, same+2, 3);
      }
    }
    next = Vector2D(pos.x, pos.y+1);
    if (!isWall(pos.x,pos.y,BOTTOM,false)){
        currently_visible(next, look, depth, leftoffset+1, rightoffset-1, same+1, 4);
    }
    next = Vector2D(pos.x+1, pos.y);
    if (!isWall(pos.x,pos.y,RIGHT,false))
      currently_visible(next, look, depth-1, leftoffset+2, rightoffset+2); 
    if (leftoffset > 1){
      next = Vector2D(pos.x, pos.y-2);
      if (!isWall(pos.x,pos.y-1,TOP,false) && !isWall(pos.x,pos.y,TOP,false)){
	  currently_visible(next, look, depth, leftoffset-1, rightoffset+1,same+2, 5);
      }
    }
    next = Vector2D(pos.x, pos.y-1);
    if (!isWall(pos.x,pos.y,TOP,false)){
        currently_visible(next, look, depth, leftoffset-1, rightoffset+1,same+1, 6);
    }
  }
  
  else if (look == TOP){
    if (rightoffset > 1){
      next = Vector2D(pos.x+2, pos.y);
      if (!isWall(pos.x+1,pos.y,RIGHT,false) && !isWall(pos.x,pos.y,RIGHT,false)){
	  currently_visible(next, look, depth, leftoffset+1,rightoffset-1,same+2, 3);
      }
    }
    next = Vector2D(pos.x+1, pos.y);
    if (!isWall(pos.x,pos.y,RIGHT,false)){
        currently_visible(next, look, depth, leftoffset+1, rightoffset-1,same+1, 4);
    }
    next = Vector2D(pos.x, pos.y-1);
    if (!isWall(pos.x,pos.y,TOP,false))
      currently_visible(next, look, depth-1, leftoffset+2, rightoffset+2);
    if (leftoffset > 1){
      next = Vector2D(pos.x-2, pos.y);
      if (!isWall(pos.x-1,pos.y,LEFT,false) && !isWall(pos.x,pos.y,LEFT,false)){
          currently_visible(next, look, depth, leftoffset-1,rightoffset+1,same+2, 5);
      }
    }
    next = Vector2D(pos.x-1, pos.y);
    if (!isWall(pos.x,pos.y,LEFT,false)){
        currently_visible(next, look, depth, leftoffset-1,rightoffset+1,same+1, 6);
    }
  }
  
  else if (look == LEFT){
    if (leftoffset > 1){
      next = Vector2D(pos.x, pos.y+2);
      if (!isWall(pos.x,pos.y+1,BOTTOM,false) && !isWall(pos.x,pos.y,BOTTOM,false)){
	currently_visible(next, look, depth, leftoffset-1,rightoffset+1,same+2, 3);
      }
    }
    next = Vector2D(pos.x, pos.y+1);
    if (!isWall(pos.x,pos.y,BOTTOM,false)){
        currently_visible(next, look, depth, leftoffset-1,rightoffset+1,same+1, 4);
    }
    next = Vector2D(pos.x-1, pos.y);
    if (!isWall(pos.x,pos.y,LEFT,false))
      currently_visible(next, look, depth-1, leftoffset+2, rightoffset+2); 
    if (rightoffset > 1){
      next = Vector2D(pos.x, pos.y-2);
      if (!isWall(pos.x,pos.y-1,TOP,false) && !isWall(pos.x,pos.y,TOP,false)){
        currently_visible(next, look, depth, leftoffset+1,rightoffset-1,same+2, 5);
      }
    }
    next = Vector2D(pos.x, pos.y-1);
    if (!isWall(pos.x,pos.y,TOP,false)){
        currently_visible(next, look, depth, leftoffset+1,rightoffset-1,same+1, 6);
    }
  }
  //These were many lines of code for the task described above...
#endif
}

//is there a wall or closed door?
bool World::isWall(short x, short y, Direction dir, bool useVisibility){
  if (x < 0 || x >= width_)
    return false;
  if (y < 0 || y >= height_)
    return false;
  Field& f = world_[y][x];
  const Door* doo = getDoor(Vector2D(x,y),dir);
  bool closedDoor = false;
  if (doo){
    closedDoor = doo->getStatus();
  }
  //The field must be active or visibility disabled(see everything) to see something
  //And there must be a wall or a closed door
  return ((f.getStatus() || !useVisibility) && (f.wallbits.test(dir) || closedDoor));
}

//This is now obsolete, getDoor does a better job.
bool World::isDoor(short x, short y, Direction dir, bool useVisibility) const {
  if (x < 0 || x >= width_)
    return false;
  if (y < 0 || y >= height_)
    return false;
  Field& f = world_[y][x];
  return ((f.getStatus() || !useVisibility) && f.doorbits.test(dir));
}

//calculates, if pos1 can see pos2
bool World::isVisible(Vector2D pos1, Vector2D pos2){
  //invalid position
  if (pos1.x < 0 || pos1.x >= width_ || pos1.y < 0 || pos1.y >= height_ ||
    pos2.x < 0 || pos2.x >= width_ || pos2.y < 0 || pos2.y >= height_)
      return false;
  //rasterize a line from pos1 to pos2 over the fields
  //sort of bresenham
  int ydiff = pos2.y - pos1.y;
  int xdiff = pos2.x - pos1.x;
  //not steep?
  if (abs(ydiff) < abs(xdiff)){
    //swap?
    if (pos2.x < pos1.x){
      Vector2D tmp = pos1;
      pos1 = pos2;
      pos2 = tmp;
    }
    Vector2D old = pos1;
    double dy = (double)ydiff/xdiff;
    double y = pos1.y+dy;
    for (int x = pos1.x+1; x <= pos2.x; x++){
      //are walls in between?
      Vector2D toTest(x, (short)(y+0.5));
      //east
      if (old.x+1 == toTest.x && old.y == toTest.y){
        if (isWall(old.x, old.y, RIGHT, false))
          return false;
      }
      //north east
      else if (old.x+1 == toTest.x && old.y+1 == toTest.y){
        if ((isWall(old.x, old.y, RIGHT, false) && isWall(old.x, old.y-1, RIGHT, false)) ||
          (isWall(old.x, old.y, TOP, false) && isWall(old.x+1, old.y, TOP, false)) ||
          (isWall(old.x, old.y, TOP, false) && isWall(old.x, old.y, RIGHT, false)) ||
          (isWall(old.x+1, old.y-1, BOTTOM, false) && isWall(old.x+1, old.y-1, LEFT, false)))
            return false;
      }
      //south east
      else{
        if ((isWall(old.x, old.y, RIGHT, false) && isWall(old.x, old.y+1, RIGHT, false)) ||
          (isWall(old.x, old.y, BOTTOM, false) && isWall(old.x+1, old.y, BOTTOM, false)) ||
          (isWall(old.x, old.y, BOTTOM, false) && isWall(old.x, old.y, RIGHT, false)) ||
          (isWall(old.x+1, old.y+1, TOP, false) && isWall(old.x+1, old.y+1, LEFT, false)))
            return false;
      }
      y += dy;
      old = toTest;
    }
    return true;
  }
  //the steep case
  else{
    //swap?
    if (pos2.y > pos1.y){
      Vector2D tmp = pos1;
      pos1 = pos2;
      pos2 = tmp;
    }
    Vector2D old = pos1;
    double dx = (double)xdiff/ydiff;
    double x = pos1.x-dx;
    for (int y = pos1.y-1; y >= pos2.y; y--){
      Vector2D toTest((short)(x+0.5), y);
      //north
      if (old.x == toTest.x && old.y-1 == toTest.y){
        if (isWall(old.x, old.y, TOP, false))
          return false;
      }
      //north east
      else if (old.x+1 == toTest.x && old.y-1 == toTest.y){
        if ((isWall(old.x, old.y, RIGHT, false) && isWall(old.x, old.y-1, RIGHT, false)) ||
          (isWall(old.x, old.y, TOP, false) && isWall(old.x+1, old.y, TOP, false)) ||
          (isWall(old.x, old.y, TOP, false) && isWall(old.x, old.y, RIGHT, false)) ||
          (isWall(old.x+1, old.y-1, BOTTOM, false) && isWall(old.x+1, old.y-1, LEFT, false)))
            return false;
      }
      //north west
      else{
        if ((isWall(old.x, old.y, LEFT, false) && isWall(old.x, old.y-1, LEFT, false)) ||
          (isWall(old.x, old.y, TOP, false) && isWall(old.x-1, old.y, TOP, false)) ||
          (isWall(old.x, old.y, TOP, false) && isWall(old.x, old.y, LEFT, false)) ||
          (isWall(old.x-1, old.y-1, BOTTOM, false) && isWall(old.x-1, old.y-1, RIGHT, false)))
            return false;
      }
      x -= dx;
      old = toTest;
    }
    return true;
  }
}

//test if a certain visibility bit is set
bool World::isTempVis(const Vector2D& pos, short bit){
  if (pos.x < 0 || pos.x >= width_ || pos.y < 0 || pos.y >= height_)
    return false;
  return world_[pos.y][pos.x].tempVis.test(bit);
}

//returns a door or NULL if there's none
Door* World::getDoor(const Vector2D& pos, Direction dir){
  if (pos.x < 0 || pos.x >= width_ || pos.y < 0 || pos.y >= height_)
    return NULL;
  if (!world_[pos.y][pos.x].doorbits.test(dir))
    return NULL;
  short idx = world_[pos.y][pos.x].dooridx[dir];
  return doors_[idx];
}

bool World::isMonsterVisible(Vector2D pos){
  vector<Field*> vf = getVisibleFields(pos);
  for (unsigned i = 0; i < vf.size(); i++){
    Vector2D fp = vf[i]->getPosition();
    //a monster?
    Monster* m =dynamic_cast<Monster*>(wrld.getObject(fp));
    if (m)
      return true;
  }
  return false;
}

//place an inventory at a certain field
void World::placeInventory(Inventory* items, Vector2D position){
  Field& f = getField(position);
  //reset inventory on that field
  if (items == NULL){
    f.items = NULL;
    return;
  }
  //does the inventory contain no items
  if(items->getItems().size() <= 1){
    return;
  }
  //no inventory present, just set it
  if (f.items == NULL){
    f.items = items;
  }
  //merge inventories
  else{
    vector<Item>& its = items->getItems();
    for (unsigned i = 0; i < its.size(); i++){
      Item& it = its[i];
      f.items->addItem(it);
    }
  }
}