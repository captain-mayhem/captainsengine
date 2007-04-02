//  ==================================================================
// |                         Shader Billard                           |
// |------------------------------------------------------------------|
// | Graphics Programming and Applications                            |
// |                                                                  |
// | Marco Staginski                                                  |
// | staginski@gmx.de                                                 |
// |------------------------------------------------------------------|
// | File: vector.hh                                                  |
//  ==================================================================

#ifndef VECTOR_HH
#define VECTOR_HH

#include <iostream>
#include <math.h>

using std::ostream;

namespace Math{

extern float Epsilon;
  
//! A 2D-Vector for integers
class Vector2D {
public:
  //! Constructor
  Vector2D();
  //! Constructor that inits with values
  Vector2D(short x, short y){ 
    this->x = x; this->y = y;
  }
  //! Destructor
  ~Vector2D();
  //! Subtract two vectors
  inline Vector2D operator-(const Vector2D& v) const {
    return Vector2D(x-v.x, y-v.y);
  }
  //! tests for inequality
  inline const bool operator!=(const Vector2D& v) const {
    return (v.x != x || v.y != y);
  }
  //! tests for equality
  inline const bool operator==(const Vector2D& v) const {
    return (v.x == x && v.y == y);
  }
  //! write vector to a stream
	friend ostream& operator<<(ostream& stream, const Vector2D& mat);
  //! the components
  short x, y;
};

inline bool operator<(Vector2D a, Vector2D b){
  return a.x < b.x && a.y < b.y;
}

ostream& operator<<(ostream& stream, const Vector2D& vec);


//! A 3D-Vector with float precision
class Vector3D {
public:
  //! Default Constructor
  //! Inits the vector with zeros
  Vector3D();
  //! Constructor that inits the Vector with x,y,z.
  Vector3D(float x, float y, float z) {
    this->x = x; this->y = y; this->z = z;
  }
  //! Constructor that inits the vector with an array
  Vector3D(float data[3]){
    x = data[0]; y = data[1]; z = data[2];
  }
  //! takes a 2D vector as initialization
  /*! y-component is set to 0
   */
  Vector3D(const Vector2D v){
    x = v.x; y = 0; z = v.y;
  }
  //! Destructor
  ~Vector3D();
  //! cast
  inline operator float*() {return data;}
  //! cast
  inline operator const float*() {return data;}
  //! Vector addition
  inline Vector3D operator+(const Vector3D& v) const{
    return Vector3D(v.x + x, v.y + y, v.z + z);
  }
  inline Vector3D operator+=(const Vector3D& v){
    x += v.x; y += v.y; z += v.z;
    return *this;
  }
  //! Vector difference
  inline Vector3D operator-(const Vector3D& v) const{
    return Vector3D(x - v.x, y - v.y, z - v.z);
  }
  //! product with vector and scalar
  inline Vector3D operator*(const float num) const{
    return Vector3D(x * num, y * num, z * num);
  }
  //! divides a vector through a scalar
  inline Vector3D operator/(const float num) const{
    return Vector3D(x / num, y / num, z / num);
  }
  //! if two vectors are inequal
  inline const bool operator!=(const Vector3D& v) const{
    return (v.x != x || v.y != y || v.z != z);
  }
  //! if two vectors are equal
  /*! This function does not test for exact equality, but
   *  takes an epsilon into consideration
   */
  inline const bool operator==(const Vector3D& v) const {
    float epsilon = 0.101f;
    return (fabs(v.x - x) < epsilon && fabs(v.y - y) < epsilon && fabs(v.z - z) < epsilon);
  }
  //! performing the cross product.
  Vector3D cross(const Vector3D& v) const;
  //! returns the length of the vector
  inline float length() const {return magnitude();}
  //! returns the magnitude of the vector
  float magnitude() const;
  //! returns the squared magnitude of the vector
  float magnitudeSquared() const;
  //! returns a normalized vector
  Vector3D normalized() const;
  //! normalizes a vector
  void normalize();
  //! returns the distance between two 3D points
  float distance(Vector3D p);
  //! performs the dot product between two vectors
  float dot(const Vector3D& v) const;
  //! returns the angle (in radians) between two vectors
  double angle(Vector3D v);
  //! write matrix to a stream
	friend ostream& operator<<(ostream& stream, const Vector3D& vec);
  //! the three conponents of the vector
  union{
    struct{
      float x;
      float y;
      float z;
    };
    float data[3];
  };
};

ostream& operator<<(ostream& stream, const Vector3D& vec);


//! A 2D-Vector for floats
class Vec2f{
public:
  //! Constructor
  Vec2f();
  //! Constructor, that inits the values
  Vec2f(float x, float y){ 
    this->x = x; this->y = y;
  }
  //! get the length of the vector
  float length() const;
  //! get the squared length of the vector
  float lengthSquared() const;
  //! Destructor
  ~Vec2f();
  float x, y;
};

}

#endif
