#ifndef MAPCHUNK_H
#define MAPCHUNK_H

#include <map>
#include "common/octree.h"
#include "common/graph.h"
#include "math/vector.h"
#include "mesh/model.h"

class MapChunk{
public:
  class Node : public Common::GraphNode{
  public:
    Node(const CGE::Vec3d& pos, int id) : mPos(pos),mId(id),mModel(NULL){}
    virtual ~Node() {}
    CGE::Vec3d mPos;
    int mId;
    MeshGeo::Model* mModel;
  };
  class Iterator{
  public:
    Iterator(std::map<int,Node*>& container){
      mCurrent = container.begin(); mLast = container.end();
    }
    bool hasNext(){
      return mCurrent != mLast;
    }
    Node* next(){
      return (*mCurrent++).second;
    }
  protected:
    std::map<int,Node*>::iterator mCurrent;
    std::map<int,Node*>::iterator mLast;
  };
  MapChunk();
  ~MapChunk();
  void setExtent(const CGE::Vec3d& minPoint, const CGE::Vec3d& maxPoint) {mMinExtent = minPoint; mMaxExtent = maxPoint;}
  void addNode(const CGE::Vec3d& position, int nodeid);
  void addStreet(int streetid, int fromNode, int toNode);
  void transformIntoPlane();
  //CGE::Vec3d getCenter(){return (mMinBox+mMaxBox)/2.0;}
  CGE::Vec3f getNormal() {return CGE::Vec3f(0,1,0);}
  Iterator getNodeIterator() {return Iterator(mNodes);}
  void render(const CGE::Camera* cam);
protected:
  static void renderOctreeCallback(Node* node);

  static MapChunk* mMap;

  std::map<int,Node*> mNodes;
  Common::Octree<double,Node*> mTree;
  Common::Graph mGraph;
  CGE::Vec3d mMinBox;
  CGE::Vec3d mMaxBox;
  CGE::Vec3d mMinExtent;
  CGE::Vec3d mMaxExtent;
  CGE::Matrix mTransform;
};

#endif
