#ifndef HQMEXPORT_H
#define HQMEXPORT_H

#include <string>
#include <vector>

#include "system/bitset.h"
#include "math/vector.h"
#include "math/matrix.h"

namespace Graphics{
class Scene;
};

class Field{
public:
  Field() {
    position = Math::Vector2D(-1,-1);
    id = -1;
    active = false;
    wallbits.setData(0);
    doorbits.setData(0);
    for (int i = 0; i < 4; i++){
      dooridx[i] = 0;
    }
  }
  //! The postion
  Math::Vector2D position;
  //! Object id
  short id;
  //! Is the field active
  bool active;
  //! Where are the walls
  System::Bitset wallbits;
  //! Where are the doors
  System::Bitset doorbits;
  //! door indices
  short dooridx[4];
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
  bool exportHQM(Graphics::Scene& scn, const std::string& filename);
private:
  //! Get the field position given a world position
  Math::Vector2D convertToMap(const Math::Vector3D& pos);
  //! Get the direction given a matrix
  Direction extractDir(const Math::Matrix& mat);
  //! the width of the map
  short width_;
  //! the height of the map
  short height_;
  //! the map
  Field** map_;
  //! the room structure
  std::vector< std::vector< Math::Vector2D > > rooms_;
};
  

#endif
