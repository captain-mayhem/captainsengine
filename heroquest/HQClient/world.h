//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: world.hh                                                   |
//  ==================================================================

#ifndef WORLD_HH
#define WORLD_HH

#include <iostream>
#include <string>

#include "math/vector.h"
#include "renderer/scene.h"
//#include "array.hh"
#include "field.h"
#include "hero.h"
#include "door.h"
#include "monster.h"
#include "furniture.h"
#include "overlay.h"

using std::string;

#define WALL_HEIGHT 16;
#define QUAD_SIZE 8;

namespace Graphics{
  class Texture;
}

//using Graphics::Texture;


//! The world in the game
/*! This class is responsible for loading and rendering the 
 * tile-based world and for bringing all gameobjects
 * together. It is of central importance for the game.
 */
class World {
public:
  //! Constructor
  World();
 
  //! Copy Constructor
  World(const World& w);
  
	//! Destructor
  ~World();

  inline void enableWallCollision(bool enable){
  	wallCollision_ = enable;
  }
  
	//! load a level file
  bool load(const string& name);
  
	//! deallocate memory
  //! deallocates all the memory that was taken for the map
  void deInit();
  
	//! renders the map
  void render();
  
	//! renders the map in 2D
  /*! \param vis should visibility taken into account or just rendered the whole map
   */
  void render2D(bool vis);
  
	//! updates the vertices for collision detection
  /*! \param modelPos the position araound which the collision vertices should be built.
   */
  void updateCollisionVertices(Vector2D modelPos);
  
	//! returns the vertices for collision
  Vector3D** getWorld() {return vWorld_;}
  
	//! returns the number of collision vertices
  int getNumberOfVerts() {return numberOfVerts_;}
  
	//! adds a hero to the world
  /*! \param heroe the hero to be added
   * \param posIdx the index of the starting position on which the hero should be placed
   */
  void addHero(const Hero& heroe, const short posIdx);
  
	//! adds a monster to the world
  /*! \param monster the monster to be added
   * \param pos the position where the monster should be placed
   */
  void addMonster(Monster* monsta, const Vector2D& pos, int vecPos);
	
  //! adds a monster to the world
  /*! \param monster the monster to be added
   * \param pos the position where the monster should be placed
   */
  void addMonster(Monster* monsta, const Vector2D& pos);
  
	//! adds a furniture to the world
  /*! \param furniture the furniture to be added
   * \param pos the position where the furniture should be placed
	 * \param d the orientation of the furniture
   */
  void addFurniture(Furniture* furniture, const Vector2D& pos, Direction d, int vecPos);
  
  //! adds an overlay to the world
  /*! \param overlay the overlay to be added
   * \param pos the position where the overlay should be placed
   * \param d the orientation of the overlay
   */
  void addOverlay(Overlay* overlay, const Vector2D& pos, Direction d, int vecPos);
  
	//! sets a game object in the world
  /*! \param o the GameObject to be set
   *  \param pos the new position for the object
   *  \param interpolate if the move from the old to the new position should be interpolated
   */
  bool setObject(GameObject* o, const Vector2D& pos, bool interpolate=false);

  //! sets a game object on another field without removing previous position
  /* ! objects that are more than one field large need this
   */
  void setObjectAnchor(GameObject* o, const Vector2D& pos);
  void setObjectAnchor(Overlay* o, const Vector2D& pos);
  
	//! returns the gameObject on position pos
  inline GameObject* getObject(const Vector2D& pos) const {
    if (pos.x >= width_ || pos.x < 0 || pos.y >= height_ || pos.y < 0) return NULL;
    return world_[pos.y][pos.x].object;
  }

  //! remove a game object from the world
  void removeObject(const Vector2D& pos);
  
	//! transforms 3D coordinates to 2D map coordinates
  inline Vector2D realToModelPos(Vector3D realPos){
		Vector2D ret;
		ret.x = (int)realPos.x / QUAD_SIZE;
		ret.y = (int)realPos.z / QUAD_SIZE;
		return ret;
  }
  
	//! transforms map coordinates to the 3D coordinates in the world
  inline Vector3D modelToRealPos(Vector2D modelPos){
    Vector3D ret;
    ret.x = (float)modelPos.x*QUAD_SIZE;
    ret.x += 0.5*QUAD_SIZE;
    ret.y = 0.8f*QUAD_SIZE;
    ret.z = (float)modelPos.y*QUAD_SIZE;
    ret.z += 0.5*QUAD_SIZE;
    return ret;
  }
  
	//! returns the number of free hero starting positions
  inline unsigned short getHeroSize() const {return heroSize_;}
  
	//! returns the heros
  inline Hero* getHeros() {return heros_;}

  //! returns the monsers
  inline vector<Monster*>& getMonsters(){return monsters_;}
  
	//! returns all hero starting positions
  inline Vector2D* getStarts() {return starts_;}
  
	//! returns map dimensions
  inline Vector2D getMapSize() {return Vector2D(width_, height_);}

  //! gets a field from the map
  inline Field& getField(const Vector2D& pos) {return world_[pos.y][pos.x];}
  
	//! returns if the world is loaded
  inline bool isLoaded() {return loaded_;}
  
	//! updates 2D map visibility
  /* \param pos the position on which the visibility should be updated
   * \param subsequent this is set to true when it is called within itself recursivly
   * \param oldPos the position that was originally specified in pos
   */
  void update_visibility(const Vector2D& pos, bool subsequent=false, Vector2D oldPos=Vector2D());
  
  //! returns a set of fields that are currently visible
  vector<Field*> getVisibleFields(const Vector2D& pos, bool subsequent=false, Vector2D oldPos=Vector2D());
  
	//! determins approximativly which fields are visible and thus have to be rendered
  /*! the algorithm determins a potential visible set (pvs) from a field. Because the information is not
   *  precalculated, the pvs can only be approximated in reasonable time.
   *  \param pos the position for which the pvs should be found
   *  \param look the direction in which the camera looks
   *  \param depth the distance of how many fields should be calculated
   *  \param leftoffset how many fields it can go left in each forward step
   *  \param rightoffset how many field it can go right in each forward step
   *  \param same how many fields were gone sideways without interruption
   *  \param idx in which bit should be stored if you were already on this position
   */
  void currently_visible(const Vector2D& pos, Direction look, short depth=10, short leftoffset=2, short rightoffset=2, short same=0, short idx=2);
  
  //void currVisHelper(const Vector2D& next, Direction look, const Vector2D& oldPos);
  
  //! determines if pos2 is visible from pos1
  /*! it uses a kind of bresenham to determine visibility
   */
  bool isVisible(Vector2D pos1, Vector2D pos2);
  
	//! gets a door
  /*! \param pos the field position
   *  \param dir the direction in which to look on the field
   *  \returns the door at specified direction or NULL if no door is present
   */
  Door* getDoor(const Vector2D& pos, Direction dir);
  
	//! tells if there is a wall at (x,y) in dir
  /*! \param x the x-position
   *  \param y the y-position
   *  \param dir the direction to look for walls
   *  \param useVisibility if only visible or all walls should be taken into account
   */
  bool isWall(short x, short y, Direction dir, bool useVisibility=true);

	//! init some data structures
	void init();

  //! if a monster is visible from position pos
  bool isMonsterVisible(Vector2D pos);

  //! when set to false, heros can pass through monsters and vice versa
  inline void setRespectClasses(bool respect){
    respectClasses_ = respect;
  }

  //! get the value of the respectClasses variable explained in setRespectClasses
  inline bool getRespectClasses(){
    return respectClasses_;
  }

  //! the scripts
  struct scriptPos{
    char script[2];
    Vector2D pos;
  };

  //! get all fields that belong to the room with the id id.
  inline vector<Field*>& getRoom(short id){return rooms_[id];}

  //! unload a level
  inline void unload() {loaded_ = false; heroSize_ = 0;}

  //! place an inventory at a certain field
  void placeInventory(Inventory* items, Vector2D position);
	
private:
  //! tells if there is a door
  /*! the function works as isWall
   */
  bool isDoor(short x, short y, Direction dir, bool useVisibility=true) const;

	//! this is needed for visibility calculation
  /*! a field is marked with a certain bit if a visibility algorithm came over it.
   */
  bool isTempVis(const Vector2D& pos, short bit);
  
  //! the width of the map
  short width_;
  
	//! the height of the map
  short height_;
  
	//! if the world is loaded
  bool loaded_;
  
	//! the world is an 2D-Field array
  Field** world_;
  
	//! the number of vertices in the collision detection
  unsigned numberOfVerts_;
  
	//! the collision detection vertices
  Vector3D** vWorld_;
  
	//! some special collision vertices
  /*! this is an invisible box around the current field if
   *  no more moves are left
   */
  Vector3D moveBox_[4];
  
	//! the number of hero starting positions	
  unsigned short heroSize_;
  
	//! the hero starting positions
  Vector2D* starts_;
  
	//! the heros in the world
  Hero* heros_;

	//! the monsters in the world
	vector<Monster*> monsters_;

	//! the furniture in the world
	vector<Furniture*> furniture_;

  //! the overlays in the world
  vector<Overlay*> overlays_;
  
	//! the fields are grouped together to rooms
  vector< vector< Field*> > rooms_;
  
	//! saves which fields are currently visible
  vector<Field*> canSee_;

  //! which fields are visible in 2D
  vector<Field*> visibleFields_;
  
  //! the doors in the world
  vector<Door*> doors_;

	//! all available furniture
	vector<Furniture*> furnitureTypes_;

  //! counts the number of monsters in the world
	vector<unsigned short> monsterCounter_;
	
	struct monsterPos{
		char monster[3];
		Vector2D pos;
	};

	struct furniturePos{
		char furniture[3];
		Vector2D pos;
	};
	
  struct overlayPos{
		char overlay[3];
		Vector2D pos;
	};

  //! should collision between walls be enabled
  bool wallCollision_;

  //! is there a difference between monsters and heros
  bool respectClasses_;

  //! the scene
  Graphics::Scene scene_;
};

extern class World wrld;

#endif
