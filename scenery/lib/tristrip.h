#ifndef SC_TRISTRIP_H
#define SC_TRISTRIP_H

#include <vector>

#include <math/vector.h>

class TriStrip{
public:
  void addVertex(CGE::Vec3f v) {mVerts.push_back(v);}
  void addVertex(float x, float y, float z=0.0f) {mVerts.push_back(CGE::Vec3f(x,y,z));}
  //void addVertex(const CGE::Vec3f& v) {mVerts.push_back(v);}
  float* getVertices() {return mVerts.size() > 0 ? &mVerts[0].data[0] : NULL;}
  unsigned getNumVertices() {return mVerts.size();}
private:
  std::vector<CGE::Vec3f> mVerts;
};

#endif
