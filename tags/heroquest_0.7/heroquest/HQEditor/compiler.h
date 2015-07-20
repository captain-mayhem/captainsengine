//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// | Graphics Programming and Applications (Assignment 5)             |
// |                                                                  |
// | Marco Staginski                                                  |
// | staginski@gmx.de                                                 |
// |------------------------------------------------------------------|
// | File: compiler.hh                                                |
//  ==================================================================

#ifndef COMPILER_HH
#define COMPILER_HH

#include <map>
#include <vector>
#include <string>
#include "math/vector.h"
#include "../HQClient/field.h"
#include "../HQClient/door.h"

using std::vector;
using std::map;
using std::string;

//! Compiles the given data into the HeroQuest .hqm file format
class Compiler {
public:
  //! Constructor
  /*! takes dimensions of the map
   */
  Compiler(short width, short height);
  //! Destructor
  ~Compiler();
  //! Add a Field to a room
  /*! \param pos the position of the Field to be added
   *  \param id the id of the room
   */
  void addRoom(const ::Math::Vector2D& pos, short id);
  //! Sets a wall at a Field
  /*! \param pos the position of the Field
   *  \param d the Direction of the wall
   */
  void setWall(const ::Math::Vector2D& pos, Direction d);
  //! Returns if there is a wall on a field in a specific direction
  bool isWall(short x, short y, Direction dir) const;
  //! Returns if there is a door on a field
  bool isDoor(short x, short y, Direction dir) const;
  //! Set a Door
  /*! \param pos the position of the Field where the door should be
   *  \param d the Direction of the Door
   *  \param isSecret if the Door is a secret Door
   */
  void setDoor(const ::Math::Vector2D& pos, Direction d, short idx);
  //! Add a hero starting position to the map
  void addStartPos(const ::Math::Vector2D& pos);
  //! Writes the compiled map to the file given by filename
  void write(string filename);

	//! Add a monster
	/*! \param pos the position of the monster
	 *  \param monster the monster type
	 */
  void addMonster(const ::Math::Vector2D& pos, char monster[3]);
	
	//! Add a furniture
	/*! \param pos the position of the furniture
	 *  \param furniture the furniture type
	 */
  void addFurniture(const ::Math::Vector2D& pos, char furniture[3]);
	
  //! Add an overlay
	/*! \param pos the position of the overlay
	 *  \param overlay the overlay type
	 */
  void addOverlay(const ::Math::Vector2D& pos, char overlay[3]);
	
	//! read in monster types
	void init();

  //! Add a script
  void addScript(char ident, const ::Math::Vector2D& pos);
private:
  //! the width of the map
  short width_;
  //! the height of the map
  short height_;
  //! the map
  //! it is just an array of Fields
  Field **map_;
  //! the room structure
  vector< vector< ::Math::Vector2D > > rooms_;
  //! the hero starting positions
  vector< ::Math::Vector2D > starts_;
  //! the doors in the map
  vector<Door> doors_;
	
	struct monsterPos{
		char monster[3];
    ::Math::Vector2D pos;
	};

	struct furniturePos{
		char furniture[3];
    ::Math::Vector2D pos;
	};

  struct overlayPos{
    char overlay[3];
    ::Math::Vector2D pos;
  };

  struct scriptPos{
    char script[2];
    ::Math::Vector2D pos;
  };

  struct Tile{
    short id;
    ::Math::Vector3D translation;
    float rotation;
  };
	
	//! the monsters
	vector<monsterPos> monsters_;

	//! the furnitures
	vector<furniturePos> furnitures_;

  //! the overlays
  vector<overlayPos> overlays_;

  //! the scripts
  vector<scriptPos> scripts_;

  //! the tiles
  vector<Tile> tiles_;
};

#endif
