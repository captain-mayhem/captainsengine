#ifndef TDMEXPORT_H
#define TDMEXPORT_H

#include <string>
#include <vector>

#include "system/bitset.h"
#include "math/vector.h"
#include "math/matrix.h"

namespace CGE{
class Scene;
};

class TDField{
public:
  TDField() {
    position = CGE::Vector2D(-1,-1);
    id = -1;
    active = false;
    wallbits.setData(0);
    doorbits.setData(0);
    for (int i = 0; i < 4; i++){
      dooridx[i] = 0;
    }
  }
  //! The postion
  CGE::Vector2D position;
  //! Object id
  short id;
  //! Is the field active
  bool active;
  //! Where are the walls
  CGE::Bitset wallbits;
  //! Where are the doors
  CGE::Bitset doorbits;
  //! door indices
  short dooridx[4];
  //! the models on that field
  std::vector<unsigned> modelids;
};

struct TDDoor{
  unsigned id;
  short type;
  CGE::Vector2D pos;
  CGE::Vector2D pos2;
};

struct Monster{
  unsigned id;
  unsigned instanceid;
  CGE::Vector2D pos;
};

struct Furniture{
  unsigned id;
  unsigned instanceid;
  CGE::Vector2D pos;
  unsigned direction;
};

struct TDOverlay{
  unsigned id;
  unsigned instanceid;
  CGE::Vector2D pos;
  unsigned direction;
};

struct TDScript{
  unsigned id;
  unsigned instanceid;
  CGE::Vector2D pos;
};

class TDMExport{
public:
  enum Direction{
    TOP,
    RIGHT,
    BOTTOM,
    LEFT
  };
  TDMExport();
  ~TDMExport();
  bool exportTDM(CGE::Scene& scn, const std::string& filename);
private:
  //! Get the field position given a world position
  CGE::Vector2D convertToMap(const CGE::Vector3D& pos, float xoffset, float yoffset);
  //! Get the direction given a matrix
  Direction extractDir(const CGE::Matrix& mat);
  //! the width of the map
  short width_;
  //! the height of the map
  short height_;
  //! the map
  TDField** map_;
  //! the room structure
  std::vector< std::vector< CGE::Vector2D > > rooms_;
  //! the starting positions
  std::vector<CGE::Vector2D> startpos_;
  //! the doors
  std::vector<TDDoor> doors_;
  //! the monsters
  std::vector<Monster> monsters_;
  //! the furniture
  std::vector<Furniture> furnitures_;
  //! the overlays
  std::vector<TDOverlay> overlays_;
  //! the scripts
  std::vector<TDScript> scripts_;
};
  

#endif
