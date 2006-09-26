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

namespace Math{

//Constructor
Vector3D::Vector3D() : x(0), y(0), z(0){
}

//Destructor
Vector3D::~Vector3D(){
}

//taking the cross product
Vector3D Vector3D::cross(const Vector3D& v) const{
  Vector3D ret; 

  ret.x = ((this->y * v.z) - (this->z * v.y));
  ret.y = ((this->z * v.x) - (this->x * v.z));
  ret.z = ((this->x * v.y) - (this->y * v.x));

  return ret;
}

//returns the magnitude of a vector
float Vector3D::magnitude() const {
  // uses the euclidian norm
  return (float)sqrt( (x * x) + (y * y) + (z * z) );
}


//returns a normalized vector
Vector3D Vector3D::normalized() const {
  float mag = magnitude();
  if (mag == 0){
    return *this;
  }
  
  float xr = x/mag;
  float yr = y/mag;
  float zr = z/mag;

  return Vector3D(xr, yr, zr);
}

//returns the distance between two 3D points
float Vector3D::distance(Vector3D p) {
  double distance = sqrt((p.x-x)*(p.x-x) + (p.y-y)*(p.y-y) + (p.z-z)*(p.z-z));
  return (float)distance;
}

//computers the dot product of 2 vectors
float Vector3D::dot(const Vector3D& v) const {
  return ( (x * v.x) + (y * v.y) + (z * v.z) );
}


//angle between vectors in radians
double Vector3D::angle(Vector3D v) {							
  float dotProduct = this->dot(v);				

  float vectorsMagnitude = magnitude() * v.magnitude() ;

  //the angle in radians between the 2 vectors
  double angle = acos( dotProduct / vectorsMagnitude );

  //is it an invalid number
#ifdef WIN32
  if(_isnan(angle))
#else
  if(isnan(angle))
#endif
     return 0;

  // Return the angle in radians
  return angle;
}

ostream& Math::operator<<(ostream& stream, const Vector3D& vec){
  stream << "( " << vec.x << " / " << vec.y << " / " << vec.z << " )";
  return stream;
}


//Constructor
Vector2D::Vector2D() : x(0), y(0) {
}

//Destructor
Vector2D::~Vector2D() {
}

//Constructor
Vec2f::Vec2f() : x(0), y(0){
}

//Destructor
Vec2f::~Vec2f(){
}


ostream& operator<<(ostream& stream, const Vector2D& vec){
  stream << "( " << vec.x << " / " << vec.y << " )";
  return stream;
}

}
