
#include "MapChunk.h"
#include "GeoGen.h"

using namespace Common;
using namespace CGE;

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

//void MapChunk::setDimensions(const CGE::Vec3d& center, const CGE::Vec3d& range){
  //mTree.init(center, range*3);
//}

void MapChunk::addNode(const CGE::Vec3d& position, int id){
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
  std::map<int,Node*>::iterator iter = mNodes.begin();
  //Vec3d v1 = (*iter++).second->mPos;
  //Vec3d v2 = (*iter++).second->mPos;
  //Vec3d v3 = (*iter++).second->mPos;
  //mPlaneNormal = ((v2-v1).cross(v3-v1)).normalized();
  //for (iter = mNodes.begin(); iter != mNodes.end(); ++iter){
  //  (*i(*iter).second->mPos;
  //}
  mPlaneNormal = center.normalized();
  CGE::Matrix planerotation(Matrix::Rotation, mPlaneNormal.cross(Vec3f(0,1,0)).normalized(), acos(mPlaneNormal.dot(Vec3f(0,1,0))));
  CGE::Matrix origintranslation(Matrix::Translation, center*-1);
  CGE::Matrix valuescale(Matrix::Scale, Vec3f(scale,scale,scale));
  CGE::Matrix transform = valuescale*planerotation*origintranslation;/*valuescale*origintranslation/**planerotation*/;
  mPlaneNormal = Vec3d(0,1,0);
  mMinBox = Vec3d(DBL_MAX,DBL_MAX,DBL_MAX);
  mMaxBox = Vec3d(-DBL_MAX,-DBL_MAX,-DBL_MAX);
  for (iter = mNodes.begin(); iter != mNodes.end(); ++iter){
    (*iter).second->mPos = transform*(*iter).second->mPos;
    box_extent(mMinBox,mMaxBox,(*iter).second->mPos);
  }

  //planerotation = planerotation.transpose();
  //mMaxBox = transform*mMaxBox;
  //mMinBox = transform*mMinBox;
  mTree.init(Vec3d(),(mMaxBox-mMinBox)/2.0+Vec3d(1,1,1));
  for (iter = mNodes.begin(); iter != mNodes.end(); ++iter){
    //(*iter).second->mPos = transform*(*iter).second->mPos;//planerotation*(((*iter).second->mPos-center)*scale);
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
    Graphics::Renderer* rend = System::Engine::instance()->getRenderer();
    rend->setColor(0.2,0.2,0.2,1.0);
    node->mModel->setupMaterial();
    node->mModel->render();
    node->mModel->resetMaterial();
  }
}

