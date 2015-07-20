#include "OSMMapAdapter.h"

#include "MapChunk.h"
#include "OSMReader.h"

using namespace CGE;

#define box_extent(minBox,maxBox,point) \
  minBox.x = point.x < minBox.x ? point.x : minBox.x; \
  minBox.y = point.y < minBox.y ? point.y : minBox.y; \
  minBox.z = point.z < minBox.z ? point.z : minBox.z; \
  maxBox.x = point.x > maxBox.x ? point.x : maxBox.x; \
  maxBox.y = point.y > maxBox.y ? point.y : maxBox.y; \
  maxBox.z = point.z > maxBox.z ? point.z : maxBox.z; \

OSMMapAdapter::OSMMapAdapter() : mMinBox(DBL_MAX,DBL_MAX,DBL_MAX), mMaxBox(-DBL_MAX,-DBL_MAX,-DBL_MAX){
}

OSMMapAdapter::~OSMMapAdapter(){

}

void OSMMapAdapter::load(std::string filename, MapChunk* map){
  OSMReader rdr(filename);
  rdr.read(this);
  buildMap(map);
}

void OSMMapAdapter::addNode(const CGE::Vec3d& position, uint64 id){
  mNodePositions[id] = position;
  box_extent(mMinBox,mMaxBox,position);
}

void OSMMapAdapter::addStreet(uint64 streetid, const std::list<uint64>& nodes, const StreetInfo& info){
  if (nodes.size() < 2)
    return;
  mStreetNodes[streetid] = nodes;
  mStreetInfo[streetid] = info;
}

void OSMMapAdapter::transformIntoPlane(){
  static const float scale = 2000; //km to m
  Vec3d center = (mMinExtent+mMaxExtent)/2.0;
  std::map<uint64,Vec3d >::iterator iter = mNodePositions.begin();
  Vec3f planeNormal = center.normalized();
  CGE::Matrix planerotation(Matrix::Rotation, planeNormal.cross(Vec3f(0,1,0)).normalized(), acos(planeNormal.dot(Vec3f(0,1,0))));
  CGE::Matrix origintranslation(Matrix::Translation, center*-1);
  CGE::Matrix valuescale(Matrix::Scale, Vec3f(scale,scale,scale));
  CGE::Matrix ytozero(Matrix::Identity); ytozero.at(5) = 0;
  CGE::Matrix offset(Matrix::Translation, Vec3f(0,0.05f,0));
  mTransform = offset*ytozero*valuescale*planerotation*origintranslation;

  mMinBox = mTransform*Vec3f(mMinBox);//mMinExtent;
  mMaxBox = mTransform*Vec3f(mMaxBox);//mMaxExtent;

  for (iter = mNodePositions.begin(); iter != mNodePositions.end(); ++iter){
    (*iter).second = mTransform*Vec3f((*iter).second);
  }
}

void OSMMapAdapter::buildMap(MapChunk* map){
  transformIntoPlane();
  map->setExtent(CGE::BBox(mMinBox, mMaxBox));
  std::map<StreetID,std::list<NodeID> >::iterator iter;
  for (iter = mStreetNodes.begin(); iter != mStreetNodes.end(); ++iter){
    StreetID strid = (*iter).first;
    MapChunk::StreetRef street = map->addStreet(mStreetInfo[strid]);
    std::list<NodeID> nodes = (*iter).second;
    std::list<NodeID>::iterator nodeiter = nodes.begin();
    NodeID from = *nodeiter;
    MapChunk::NodeRef fromNode = mMapNodes[from];
    if (fromNode == NULL){
      fromNode = map->addNode(mNodePositions[from]);
      mMapNodes[from] = fromNode;
    }
    NodeID to;
    MapChunk::NodeRef toNode;
    for (++nodeiter; nodeiter != nodes.end(); ++nodeiter){
      to = *nodeiter;
      toNode = mMapNodes[to];
      if (toNode == NULL){
        toNode = map->addNode(mNodePositions[to]);
        mMapNodes[to] = toNode;
      }
      map->addStreetSegment(fromNode, toNode, street);
      from = to;
      fromNode = toNode;
    }
  }
}

