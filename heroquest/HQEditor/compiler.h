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
#include "renderer/scene.h"
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
  void addRoom(const ::CGE::Vector2D& pos, short id);
  //! Sets a wall at a Field
  /*! \param pos the position of the Field
   *  \param d the Direction of the wall
   */
  void setWall(const ::CGE::Vector2D& pos, Direction d);
  //! Returns if there is a wall on a field in a specific direction
  bool isWall(short x, short y, Direction dir) const;
  //! Returns if there is a door on a field
  bool isDoor(short x, short y, Direction dir) const;
  //! Set a Door
  /*! \param pos the position of the Field where the door should be
   *  \param d the Direction of the Door
   *  \param isSecret if the Door is a secret Door
   */
  void setDoor(const ::CGE::Vector2D& pos, Direction d, short idx);
  //! Add a hero starting position to the map
  void addStartPos(const ::CGE::Vector2D& pos);
  //! Writes the compiled map to the file given by filename
  void write(string filename);

	//! Add a monster
	/*! \param pos the position of the monster
	 *  \param monster the monster type
	 */
  void addMonster(const ::CGE::Vector2D& pos, char monster[3]);
	
	//! Add a furniture
	/*! \param pos the position of the furniture
	 *  \param furniture the furniture type
	 */
  void addFurniture(const ::CGE::Vector2D& pos, char furniture[3]);
	
  //! Add an overlay
	/*! \param pos the position of the overlay
	 *  \param overlay the overlay type
	 */
  void addOverlay(const ::CGE::Vector2D& pos, char overlay[3]);
	
	//! read in monster types
	void init();

  //! Add a script
  void addScript(char ident, const ::CGE::Vector2D& pos);

  //! Set the current (ascii) map
  void setMap(char** map) {map_ = map;}

private:
  //! load or get a mesh
  MeshGeo::Mesh* getMesh(std::string tilename, std::string path);
  //! load or get a texture
  CGE::Texture* getTexture(std::string texname);
  /*! get the info which side has to be longer
    \result 0 nothing is longer
            1 left is longer
            2 right is longer
            3 left and right is longer
  */
  short getLengthInfo(const Vector2D& pos, Direction d, CGE::Matrix* mat = NULL);
  //! the scene
  CGE::Scene scene_;
  //! the hero starting positions
  vector< ::CGE::Vector2D > starts_;
  //! the doors in the map
  vector<Door> doors_;
	
	struct monsterPos{
		char monster[3];
    ::CGE::Vector2D pos;
	};

	struct furniturePos{
		char furniture[3];
    ::CGE::Vector2D pos;
	};

  struct overlayPos{
    char overlay[3];
    ::CGE::Vector2D pos;
  };

  struct scriptPos{
    char script[2];
    ::CGE::Vector2D pos;
  };

  struct Tile{
    short id;
    ::CGE::Vector3D translation;
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

  //! the ascii map
  char** map_;

  //! the width
  short width_;

  //! the height
  short height_;
};

#endif
