#include "OSMMapAdapter.h"

#include "MapChunk.h"

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

void OSMMapAdapter::addNode(const CGE::Vec3d& position, uint64 id){
  //Ptr<Node> node = Ptr<Node>(new Node(position,id));
  mNodePositions[id] = position;
  //mGraph.addSingleNode(node);
  box_extent(mMinBox,mMaxBox,position);
}

void OSMMapAdapter::addStreet(uint64 streetid, const std::list<uint64>& nodes, const StreetInfo& info){
  if (nodes.size() < 2)
    return;
  mStreetNodes[streetid] = nodes;
  mStreetInfo[streetid] = info;
  /*
  std::list<uint64>::const_iterator iter = nodes.begin();
  uint64 from = *iter;
  uint64 to;
  for (++iter; iter != nodes.end(); ++iter){
    to = *iter;
    Ptr<Node> fromNode = (*mNodes.find(from)).second;
    Ptr<Node> toNode = (*mNodes.find(to)).second;
    mGraph.connect(fromNode,toNode);
    mGraph.connect(toNode,fromNode);
    from = to;
  }*/
}

void OSMMapAdapter::transformIntoPlane(){
  static const float scale = 1000; //km to m
  Vec3d center = (mMinExtent+mMaxExtent)/2.0;
  std::map<uint64,Vec3d >::iterator iter = mNodePositions.begin();
  Vec3f planeNormal = center.normalized();
  CGE::Matrix planerotation(Matrix::Rotation, planeNormal.cross(Vec3f(0,1,0)).normalized(), acos(planeNormal.dot(Vec3f(0,1,0))));
  CGE::Matrix origintranslation(Matrix::Translation, center*-1);
  CGE::Matrix valuescale(Matrix::Scale, Vec3f(scale,scale,scale));
  CGE::Matrix ytozero(Matrix::Identity); ytozero.at(5) = 0;
  CGE::Matrix offset(Matrix::Translation, Vec3f(0,0.05f,0));
  mTransform = offset*ytozero*valuescale*planerotation*origintranslation;

  /*mMinBox = Vec3d(DBL_MAX,DBL_MAX,DBL_MAX);
  mMaxBox = Vec3d(-DBL_MAX,-DBL_MAX,-DBL_MAX);
  for (iter = mNodes.begin(); iter != mNodes.end(); ++iter){
  box_extent(mMinBox,mMaxBox,(*iter).second->mPos);
  }*/
  mMinBox = mTransform*mMinBox;//mMinExtent;
  mMaxBox = mTransform*mMaxBox;//mMaxExtent;

  //planerotation = planerotation.transpose();
  //mMaxBox = transform*mMaxBox;
  //mMinBox = transform*mMinBox;
  //mStreets.init(Vec3d(),(mMaxBox-mMinBox)/2.0+Vec3d(1,1,1), Vec3f(10,0,10));
  //mTree.init(Vec3d(),(mMaxBox-mMinBox)/2.0+Vec3d(1,1,1));
  for (iter = mNodePositions.begin(); iter != mNodePositions.end(); ++iter){
    (*iter).second = mTransform*(*iter).second;
    //mTree.insert((*iter).second->mPos, (*iter).second);
  }
  //mTree.buildDebugVertexBuffer();
}

void OSMMapAdapter::buildMap(MapChunk* map){
  transformIntoPlane();
  map->setExtent(CGE::BBox(mMinBox, mMaxBox));
  //OSMMapAdapter::Iterator iter = getNodeIterator();
  //while (iter.hasNext()){
  std::map<StreetID,std::list<NodeID> >::iterator iter;
  for (iter = mStreetNodes.begin(); iter != mStreetNodes.end(); ++iter){
    StreetID strid = (*iter).first;
    std::list<NodeID> nodes = (*iter).second;
    std::list<NodeID>::iterator nodeiter = nodes.begin();
    NodeID from = *nodeiter;
    Ptr<MapChunk::Node> fromNode = mMapNodes[from];
    if (fromNode == NULL){
      fromNode = map->addNode(mNodePositions[from]);
      mMapNodes[from] = fromNode;
    }
    NodeID to;
    Ptr<MapChunk::Node> toNode;
    for (++nodeiter; nodeiter != nodes.end(); ++nodeiter){
      to = *nodeiter;
      toNode = mMapNodes[to];
      if (toNode == NULL){
        toNode = map->addNode(mNodePositions[to]);
        mMapNodes[to] = toNode;
      }
      map->addStreet(fromNode, toNode, NULL);
      from = to;
      fromNode = toNode;
    }
  }
}

