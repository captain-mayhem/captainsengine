#ifndef HQMEXPORT_H
#define HQMEXPORT_H

#include <string>
#include "system/bitset.h"
#include "math/vector.h"
#include "math/matrix.h"

namespace Graphics{
class Scene;
};

class Field{
public:
  //! The postion
  Math::Vector2D position;
  //! Object id
  short id;
  //! Where are the walls
  System::Bitset wallbits;
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
};
  

#endif
