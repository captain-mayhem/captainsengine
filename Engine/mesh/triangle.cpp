#include "triangle.h"

using namespace MeshGeo;


Triangle::Triangle(int v0, int v1, int v2, int t0, int t1, int t2, Edge* e0_, Edge* e1_, Edge* e2_){
  data[0]=v0;data[1]=v1;data[2]=v2;
  texdata[0]=t0;texdata[1]=t1;texdata[2]=t2;
  e0=e0_;e1=e1_;e2=e2_;
}

void TriangleList::insertTriangle(Triangle* tri){
  push_back(tri);
}

// Returns the edge with endpoints v0 and v1
Edge* EdgeList::getEdge(int v0, int v1){
  if (size()==0)
    return NULL;

  for(iterator it = begin(); it < end(); it++){
    if (v0 == (*it)->v0 && v1 == (*it)->v1 || v1 == (*it)->v0 && v0 == (*it)->v1)
      return (*it);
  }

  return NULL;
}

// insert an edge
void EdgeList::insertEdge(Edge* e){
  push_back(e);
}
