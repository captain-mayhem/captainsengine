#ifndef HQMEXPORT_H
#define HQMEXPORT_H

#include <string>
#include <vector>

#include "system/bitset.h"
#include "math/vector.h"
#include "math/matrix.h"

namespace CGE{
class Scene;
};

class HQField{
public:
  HQField() {
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

struct HQDoor{
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

struct HQOverlay{
  unsigned id;
  unsigned instanceid;
  CGE::Vector2D pos;
  unsigned direction;
};

class HQMExport{
public:
  enum Direction{
    TOP,
    RIGHT,
    BOTTOM,
    LEFT
  };
  HQMExport();
  ~HQMExport();
  bool exportHQM(CGE::Scene& scn, const std::string& filename);
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
  HQField** map_;
  //! the room structure
  std::vector< std::vector< CGE::Vector2D > > rooms_;
  //! the starting positions
  std::vector<CGE::Vector2D> startpos_;
  //! the doors
  std::vector<HQDoor> doors_;
  //! the monsters
  std::vector<Monster> monsters_;
  //! the furniture
  std::vector<Furniture> furnitures_;
  //! the overlays
  std::vector<HQOverlay> overlays_;
};
  

#endif
