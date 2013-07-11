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

namespace CGE{

extern float Epsilon;

template<typename T>
class Vec2{
public:
  //! Constructor
  Vec2() : x(0), y(0){}
  //! Constructor, that inits the values
  Vec2(T x, T y){ 
    this->x = x; this->y = y;
  }
  template<typename S>
  Vec2(const Vec2<S>& v) {x = (T)v.x; y = (T)v.y;}
  //! get the length of the vector
  float length() const{
    return sqrt((float)(x*x+y*y));
  }
  //! get the squared length of the vector
  float lengthSquared() const{
    return x*x+y*y;
  }
  //! Subtract two vectors
  Vec2 operator-(const Vec2& v) const {
    return Vec2(x-v.x, y-v.y);
  }
  Vec2 operator+(const Vec2& v) const {
    return Vec2(x+v.x, y+v.y);
  }
  Vec2& operator+=(const Vec2& v){
    x += v.x;
    y += v.y;
    return *this;
  }
  Vec2 operator*(float num) const{
    Vec2 result;
    result.x = (T)(x*num);
    result.y = (T)(y*num);
    return result;
  }
  Vec2 operator*(int num) const{
    Vec2 result;
    result.x = (T)(x*num);
    result.y = (T)(y*num);
    return result;
  }
  Vec2 operator/(int num) const{
    Vec2 result;
    result.x = x/num;
    result.y = y/num;
    return result;
  }
  //! tests for inequality
  const bool operator!=(const Vec2& v) const {
    return (v.x != x || v.y != y);
  }
  //! tests for equality
  inline const bool operator==(const Vec2& v) const {
    return (v.x == x && v.y == y);
  }
  bool operator<(const Vec2& v) const{
    if (x == v.x)
      return y < v.y;
    return x < v.x;
  }
  bool operator>(const Vec2& v) const{
    if (x == v.x)
      return y > v.y;
    return x > v.x;
  }
  //! write vector to a stream
  friend std::ostream& operator<<(std::ostream& stream, const Vec2& mat);

  Vec2& normalize(){
    float len = length();
    if (len != 0.0){
      x /= len;
      y /= len;
    }
    return *this;
  }

  Vec2 cross() const{
    return Vec2(y, -x);
  }

  union{
    struct{
      T x;
      T y;
    };
    T data[2];
  };
};

typedef Vec2<short> Vector2D; //for compatibility reasons

typedef Vec2<float> Vec2f;
typedef Vec2<int> Vec2i;

/*inline bool operator<(Vector2D a, Vector2D b){
  return a.x < b.x && a.y < b.y;
}*/

std::ostream& operator<<(std::ostream& stream, const Vector2D& vec);

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

  template<typename S> Vec3(const Vec2<S>& v){
    x = (T)v.x; y = (T)v.y; z = 0;
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
  template <typename S> friend std::ostream& operator<<(std::ostream& stream, const Vec3& vec);
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
std::ostream& operator<<(std::ostream& stream, const Vec3<T>& vec);

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
std::ostream& operator<<(std::ostream& stream, const Vec3<T>& vec){
  stream << "( " << vec.x << " / " << vec.y << " / " << vec.z << " )";
  return stream;
}


typedef Vec3<float> Vector3D;//for compatibility reasons

typedef Vec3<double> Vec3d;
typedef Vec3<float> Vec3f;
typedef Vec3<int> Vec3i;

}

#endif
