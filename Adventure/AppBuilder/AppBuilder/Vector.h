#ifndef VECTOR_H
#define VECTOR_H

#include <cmath>

struct Vec2i{
  Vec2i() : x(0), y(0) {}
  Vec2i(int x, int y) {this->x = x; this->y = y;}
  Vec2i operator+(const Vec2i& v) const{
    Vec2i result;
    result.x = v.x+x;
    result.y = v.y+y;
    return result;
  }
  Vec2i& operator+=(const Vec2i& v){
    x += v.x;
    y += v.y;
    return *this;
  }
  Vec2i operator-(const Vec2i& v) const{
    Vec2i result;
    result.x = x-v.x;
    result.y = y-v.y;
    return result;
  }
  Vec2i operator*(int num) const{
    Vec2i result;
    result.x = x*num;
    result.y = y*num;
    return result;
  }
  Vec2i operator/(int num) const{
    Vec2i result;
    result.x = x/num;
    result.y = y/num;
    return result;
  }
  float length(){
    return sqrt((float)x*x+y*y);
  }
  bool operator<(const Vec2i& v) const{
    if (x == v.x)
      return y < v.y;
    return x < v.x;
  }
  bool operator>(const Vec2i& v) const{
    if (x == v.x)
      return y > v.y;
    return x > v.x;
  }
  bool operator==(const Vec2i& v) const{
    return x==v.x && y==v.y;
  }
  bool operator!=(const Vec2i& v) const{
    return x!=v.x || y!=v.y;
  }
  int x;
  int y;
};

struct Vec2f{
  Vec2f() : x(0), y(0) {}
  Vec2f(float x, float y) {this->x = x; this->y = y;}
  float x;
  float y;
};

#endif