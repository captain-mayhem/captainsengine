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
    Node(const Math::Vec3d& pos, int id) : mPos(pos),mId(id),mModel(NULL){}
    virtual ~Node() {}
    Math::Vec3d mPos;
    int mId;
    MeshGeo::Model* mModel;
  };
  MapChunk();
  ~MapChunk();
  //void setDimensions(const Math::Vec3d& center, const Math::Vec3d& range);
  void addNode(const Math::Vec3d& position, int nodeid);
  void addStreet(int streetid, int fromNode, int toNode);
  void render(const Graphics::Camera* cam);
  void buildAccelerationStructures();
  Math::Vec3d getCenter(){return (mMinBox+mMaxBox)/2.0;}
  Math::Vec3f getNormal() {return mPlaneNormal;}
protected:
  static void renderOctreeCallback(Node* node);

  static MapChunk* mMap;

  std::map<int,Node*> mNodes;
  Common::Octree<double,Node*> mTree;
  Common::Graph mGraph;
  Math::Vec3d mMinBox;
  Math::Vec3d mMaxBox;
  Math::Vec3f mPlaneNormal;
};

#endif
