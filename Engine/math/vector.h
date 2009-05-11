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
#include <float.h>

using std::ostream;

namespace CGE{

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

/*
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
  //! y-component is set to 0
   //
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
  //! This function does not test for exact equality, but
   /  takes an epsilon into consideration
   //
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
*/

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

template <typename T>
class Vec3{
public:
  //! Default Constructor
  //! Inits the vector with zeros
  Vec3();
  //! Constructor that inits the Vector with x,y,z.
  Vec3(T x, T y, T z) {
    this->x = x; this->y = y; this->z = z;
  }
  //! Constructor that inits the vector with an array
  Vec3(T data[3]){
    x = data[0]; y = data[1]; z = data[2];
  }
  //! takes a 2D vector as initialization
  /*! y-component is set to 0
  */
  Vec3(const Vector2D v){
    x = v.x; y = 0; z = v.y;
  }

  template<typename S> Vec3(const S& v){
    x = (T)v.x; y = (T)v.y; z = (T)v.z;
  }

  //! Destructor
  ~Vec3();
  //! cast
  //operator T*() {return data;}
  //! cast
  //operator const T*() {return data;}
  //! Vector addition
  Vec3 operator+(const Vec3& v) const{
    return Vec3(v.x + x, v.y + y, v.z + z);
  }
  Vec3 operator+=(const Vec3& v){
    x += v.x; y += v.y; z += v.z;
    return *this;
  }
  //! Vector difference
  Vec3 operator-(const Vec3& v) const{
    return Vec3(x - v.x, y - v.y, z - v.z);
  }
  //! product with vector and scalar
  Vec3 operator*(const T num) const{
    return Vec3(x * num, y * num, z * num);
  }
  //! product with vector and scalar
  Vec3& operator*=(const T num){
    x *= num; y *= num; z *= num;
    return *this;
  }
  //! component-wise product with vector and vector
  Vec3 operator*(const Vec3& v) const{
    return Vec3(x*v.x, y*v.y, z*v.z);
  }
  //! divides a vector through a scalar
  Vec3 operator/(const T num) const{
    return Vec3(x / num, y / num, z / num);
  }
  //! if two vectors are inequal
  const bool operator!=(const Vec3& v) const{
    return (v.x != x || v.y != y || v.z != z);
  }
  //! if two vectors are equal
  /*! This function does not test for exact equality, but
  *  takes an epsilon into consideration
  */
  inline const bool operator==(const Vec3& v) const {
    float epsilon = 0.101f;
    return (fabs(v.x - x) < epsilon && fabs(v.y - y) < epsilon && fabs(v.z - z) < epsilon);
  }

  inline bool operator<(const Vec3& v) const {
    if (x == v.x){
      if (y == v.y){
        return z < v.z;
      }
      return y < v.y;
    }
    return x < v.x;
  }

  inline bool operator>(const Vec3& v) const {
    if (x == v.x){
      if (y == v.y){
        return z > v.z;
      }
      return y > v.y;
    }
    return x > v.x;
  }

  T& operator[](int i){return data[i];}

  //! performing the cross product.
  Vec3 cross(const Vec3& v) const;
  //! returns the length of the vector
  T length() const {return magnitude();}
  //! returns the magnitude of the vector
  T magnitude() const;
  //! returns the squared magnitude of the vector
  T magnitudeSquared() const;
  //! returns a normalized vector
  Vec3 normalized() const;
  //! normalizes a vector
  void normalize();
  //! returns the distance between two 3D points
  T distance(Vec3 p);
  //! performs the dot product between two vectors
  T dot(const Vec3& v) const;
  //! returns the angle (in radians) between two vectors
  double angle(Vec3 v);
  //! write matrix to a stream
  template <typename S> friend ostream& operator<<(ostream& stream, const Vec3& vec);
  //! the three conponents of the vector
  union{
    struct{
      T x;
      T y;
      T z;
    };
    T data[3];
  };

};

template <typename T>
ostream& operator<<(ostream& stream, const Vec3<T>& vec);

template <typename T>
Vec3<T>::Vec3(): x(0), y(0), z(0) {}

template <typename T>
Vec3<T>::~Vec3(){}

template <typename T>
Vec3<T> Vec3<T>::cross(const Vec3<T>& v) const{
  Vec3<T> ret; 

  ret.x = ((this->y * v.z) - (this->z * v.y));
  ret.y = ((this->z * v.x) - (this->x * v.z));
  ret.z = ((this->x * v.y) - (this->y * v.x));

  return ret;
}

template <typename T>
T Vec3<T>::magnitude() const {
  // uses the euclidian norm
  return sqrt(x*x + y*y + z*z);
}

template <typename T>
T Vec3<T>::magnitudeSquared() const {
  return x*x + y*y + z*z;
}


template <typename T>
Vec3<T> Vec3<T>::normalized() const {
  T mag = magnitude();
  if (mag == 0){
    return *this;
  }

  T xr = x/mag;
  T yr = y/mag;
  T zr = z/mag;

  return Vec3<T>(xr, yr, zr);
}

template <typename T>
void Vec3<T>::normalize(){
  T mag = magnitude();
  if (mag != 0.0){
    x /= mag;
    y /= mag;
    z /= mag;
  }
}

template <typename T>
T Vec3<T>::distance(Vec3<T> p) {
  T distance = sqrt((p.x-x)*(p.x-x) + (p.y-y)*(p.y-y) + (p.z-z)*(p.z-z));
  return distance;
}

template <typename T>
T Vec3<T>::dot(const Vec3<T>& v) const {
  return ( (x * v.x) + (y * v.y) + (z * v.z) );
}

template <typename T>
double Vec3<T>::angle(Vec3<T> v) {							
  T dotProduct = this->dot(v);				

  T vectorsMagnitude = magnitude() * v.magnitude() ;

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

template <typename T>
ostream& operator<<(ostream& stream, const Vec3<T>& vec){
  stream << "( " << vec.x << " / " << vec.y << " / " << vec.z << " )";
  return stream;
}


typedef Vec3<float> Vector3D;//for compatibility reasons

typedef Vec3<double> Vec3d;
typedef Vec3<float> Vec3f;
typedef Vec3<int> Vec3i;

}

#endif
