#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "../math/vector.h"
#include <vector>

namespace CGE{

class Edge;

//! Just a triangle
class Triangle{
public:
  Triangle(){v0=-1;v1=-1;v2=-1;e0=NULL;e1=NULL;e2=NULL;t0=-1;t1=-1;t2=-1;}
  Triangle(int v0, int v1, int v2, int t0, int t1, int t2, Edge* e0_, Edge* e1_, Edge* e2_);
  ~Triangle(){}
  union{
    int data[3];
    struct{
      int v0;
      int v1;
      int v2;
    };
  };
  union{
    int texdata[3];
    struct{
      int t0;
      int t1;
      int t2;
    };
  };
  Edge* e0;
  Edge* e1;
  Edge* e2;
};

class TriangleList : public std::vector<Triangle*>{
public:
  void insertTriangle(Triangle* tri);
};

class Edge{
public:
  Edge(){v0=-1; v1=-1;}

  Edge(int _v0, int _v1){if(_v0 < _v1) {v0=_v0; v1=_v1;} else{v0=_v1;v1=_v0;}}
  
  //! the indices of the vertices
  int v0, v1;

  //! the adjacent triangles
  TriangleList tList;
};

class EdgeList : public std::vector<Edge*>{
public:
  //! Returns the edge with endpoints v0 and v1
  Edge* getEdge(int v0, int v1);

  //! insert an edge
  void insertEdge(Edge* e);
};

class Vertex : public CGE::Vector3D{
public:
  Vertex(){x=0.0f;y=0.0f;z=0.0f;}
  Vertex(float x, float y, float z){data[0]=x;data[1]=y;data[2]=z;}
  
  //! All edges with this vertex as endpoint
  EdgeList eList;
};

}

#endif
