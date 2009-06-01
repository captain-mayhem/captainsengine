#ifndef OSMMAPADAPTER_H
#define OSMMAPADAPTER_H

#include <map>

#include <math/vector.h>
#include <math/matrix.h>
#include <common/graph.h>

#include "MapChunk.h"
#include "MapInfo.h"

class OSMMapAdapter{
public:
  typedef uint64 StreetID;
  typedef uint64 NodeID;

  OSMMapAdapter();
  ~OSMMapAdapter();
  void load(std::string filename, MapChunk* map);
  void setExtent(const CGE::Vec3d& minPoint, const CGE::Vec3d& maxPoint) {mMinExtent = minPoint; mMaxExtent = maxPoint;}
  void addNode(const CGE::Vec3d& position, uint64 nodeid);
  void addStreet(uint64 streetid, const std::list<uint64>& nodes, const StreetInfo& info);
  void buildMap(MapChunk* map);
  void transformIntoPlane();
  CGE::Vec3f getNormal() {return CGE::Vec3f(0,1,0);}
protected:
  std::map<NodeID,CGE::Vec3d > mNodePositions;
  std::map<StreetID,std::list<NodeID> > mStreetNodes;
  std::map<StreetID,StreetInfo> mStreetInfo;
  CGE::Vec3d mMinBox;
  CGE::Vec3d mMaxBox;
  CGE::Vec3d mMinExtent;
  CGE::Vec3d mMaxExtent;
  CGE::Matrix mTransform;
  std::map<NodeID, CGE::Ptr<MapChunk::Node> > mMapNodes;
};

#endif
