//  ==================================================================
// |                         Shader Billard                           |
// |------------------------------------------------------------------|
// | Graphics Programming and Applications                            |
// |                                                                  |
// | Marco Staginski                                                  |
// | staginski@gmx.de                                                 |
// |------------------------------------------------------------------|
// | File: vector.cpp                                                 |
//  ==================================================================

#include <math.h>
#include <float.h>
#include "vector.h"

namespace CGE{

float Epsilon = (float)1.0e-5;

std::ostream& operator<<(std::ostream& stream, const Vector2D& vec){
  stream << "( " << vec.x << " / " << vec.y << " )";
  return stream;
}

}
