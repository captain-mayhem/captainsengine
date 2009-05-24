#ifndef MAPCHUNK_H
#define MAPCHUNK_H

#include <map>
#include "common/octree.h"
#include "common/OctreeStatic.h"
#include "common/graph.h"
#include "math/vector.h"
#include "mesh/model.h"

#include "MapInfo.h"

namespace CGE{
class SimpleMesh;
}

class MapChunk{
public:
  class Node : public CGE::GraphNode{
  public:
    Node(const CGE::Vec3f& pos) : mPos(pos){}
    Node(const Node& node) : CGE::GraphNode(node) {mPos = node.mPos;}
    virtual ~Node(){}
    CGE::Vec3f mPos;
  };

  class Iterator{
  public:
    Iterator(const CGE::Graph& grph): mIter(grph) {}
    bool hasNext() {return mIter.hasNext();}
    CGE::Ptr<Node> next(){return (CGE::Ptr<Node>)mIter.next();}
  protected:
    CGE::Graph::Iterator mIter;
  };

  MapChunk();
  ~MapChunk();
  void setExtent(const CGE::BBox& box) {mStreets.init(CGE::Vec3f(), box.getSpan()+CGE::Vec3f(1,1,1), CGE::Vec3f(10,0,10));}
  CGE::Ptr<Node> addNode(const CGE::Vec3f& position);
  void addStreet(CGE::Ptr<Node> from, CGE::Ptr<Node> to, CGE::Ptr<StreetInfo> info);
  //CGE::Vec3d getCenter(){return (mMinBox+mMaxBox)/2.0;}
  CGE::Vec3f getNormal() {return CGE::Vec3f(0,1,0);}
  Iterator getStreetNodeIterator() {return Iterator(mGraph);}
  void render(const CGE::Camera* cam);
  CGE::OctreeStatic<CGE::SimpleMesh*>& getStreets() {return mStreets;}
protected:
  static CGE::OctreeStatic<CGE::SimpleMesh*>::TraversalState renderCB(const std::vector<CGE::SimpleMesh*>& values, const CGE::BBox& box, uint8 flags, void* data);

  CGE::Graph mGraph;
  CGE::OctreeStatic<CGE::SimpleMesh*> mStreets;
};

#endif
