
#include "MapChunk.h"
#include "GeoGen.h"

using namespace Common;

#define box_extent(minBox,maxBox,point) \
  minBox.x = point.x < minBox.x ? point.x : minBox.x; \
  minBox.y = point.y < minBox.y ? point.y : minBox.y; \
  minBox.z = point.z < minBox.z ? point.z : minBox.z; \
  maxBox.x = point.x > maxBox.x ? point.x : maxBox.x; \
  maxBox.y = point.y > maxBox.y ? point.y : maxBox.y; \
  maxBox.z = point.z > maxBox.z ? point.z : maxBox.z; \

MapChunk* MapChunk::mMap;

MapChunk::MapChunk() : mMinBox(DBL_MAX,DBL_MAX,DBL_MAX), mMaxBox(-DBL_MAX,-DBL_MAX,-DBL_MAX){
  mTree.setRenderCallback(renderOctreeCallback);
}

MapChunk::~MapChunk(){

}

//void MapChunk::setDimensions(const Math::Vec3d& center, const Math::Vec3d& range){
  //mTree.init(center, range*3);
//}

void MapChunk::addNode(const Math::Vec3d& position, int id){
  Node* node = new Node(position,id);
  mNodes[id] = node;
  mGraph.addSingleNode(node);
  box_extent(mMinBox,mMaxBox,position);
}

void MapChunk::addStreet(int streetid, int fromNode, int toNode){
  Node* from = (*mNodes.find(fromNode)).second;
  Node* to = (*mNodes.find(toNode)).second;
  mGraph.connect(from,to);
  mGraph.connect(to,from);
}

int drawCount;

void MapChunk::render(const Graphics::Camera* cam){
  mMap = this;
  mTree.renderDebug();
  drawCount = 0;
  mTree.render(cam);
  char tmp[32];
  sprintf(tmp,"NumDraws: %i", drawCount);
  System::Engine::instance()->getFont(0)->print(20,700,tmp,0);
}

void MapChunk::buildAccelerationStructures(){
  static const int scale = 1000;
  Vec3d center = (mMinBox+mMaxBox)/2.0;
  mPlaneNormal = center.normalized();
  mTree.init(Vec3d(),(mMaxBox*scale-mMinBox*scale)/2.0);
  std::map<int,Node*>::iterator iter;
  for (iter = mNodes.begin(); iter != mNodes.end(); ++iter){
    (*iter).second->mPos = ((*iter).second->mPos-center)*scale;
    mTree.insert((*iter).second->mPos, (*iter).second);
  }
  mTree.buildDebugVertexBuffer();
}

void MapChunk::renderOctreeCallback(MapChunk::Node* node){
  ++drawCount;
  //no geometry generated, so do it now
  if (node->mModel == NULL){
    GeoGen::addJob(mMap,node);
  }
  else{
    node->mModel->setupMaterial();
    node->mModel->render();
    node->mModel->resetMaterial();
  }
}

