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
  MapChunk();
  ~MapChunk();
  //void setDimensions(const CGE::Vec3d& center, const CGE::Vec3d& range);
  void addNode(const CGE::Vec3d& position, int nodeid);
  void addStreet(int streetid, int fromNode, int toNode);
  void render(const Graphics::Camera* cam);
  void buildAccelerationStructures();
  //CGE::Vec3d getCenter(){return (mMinBox+mMaxBox)/2.0;}
  CGE::Vec3f getNormal() {return mPlaneNormal;}
protected:
  static void renderOctreeCallback(Node* node);

  static MapChunk* mMap;

  std::map<int,Node*> mNodes;
  Common::Octree<double,Node*> mTree;
  Common::Graph mGraph;
  CGE::Vec3d mMinBox;
  CGE::Vec3d mMaxBox;
  CGE::Vec3f mPlaneNormal;
};

#endif
