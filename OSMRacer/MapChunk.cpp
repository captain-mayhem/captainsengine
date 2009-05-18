
#include "MapChunk.h"
#include "GeoGen.h"
#include <mesh/SimpleMesh.h>

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
  mStreets.setTraversalCB(renderCB);
}

MapChunk::~MapChunk(){

}

//void MapChunk::setDimensions(const CGE::Vec3d& center, const CGE::Vec3d& range){
  //mTree.init(center, range*3);
//}

void MapChunk::addNode(const CGE::Vec3d& position, int id){
  Ptr<Node> node = Ptr<Node>(new Node(position,id));
  mNodes[id] = node;
  mGraph.addSingleNode(node);
  box_extent(mMinBox,mMaxBox,position);
}

void MapChunk::addStreet(int streetid, int fromNode, int toNode){
  Ptr<Node> from = (*mNodes.find(fromNode)).second;
  Ptr<Node> to = (*mNodes.find(toNode)).second;
  mGraph.connect(from,to);
  mGraph.connect(to,from);
}

int drawCount;

void MapChunk::render(const CGE::Camera* cam){
  mMap = this;
  //mTree.renderDebug();
  drawCount = 0;
  //mTree.render(cam);s
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  CGE::Material mat;
  mat.setDiffuse(CGE::Color(0.2f,0.2f,0.2f,1.0f));
  mat.setAmbient(CGE::Color(0.1f,0.1f,0.1f,1.0f));
  rend->setMaterial(mat);
  mStreets.traverse((void*)cam);
  char tmp[32];
  sprintf(tmp,"Map nodes rendered: %i", drawCount);
  CGE::Engine::instance()->getFont(0)->print(20,700,tmp,0);
}

void MapChunk::transformIntoPlane(){
  static const int scale = 1000; //km to m
  Vec3d center = (mMinExtent+mMaxExtent)/2.0;
  std::map<int,Ptr<Node> >::iterator iter = mNodes.begin();
  Vec3f planeNormal = center.normalized();
  CGE::Matrix planerotation(Matrix::Rotation, planeNormal.cross(Vec3f(0,1,0)).normalized(), acos(planeNormal.dot(Vec3f(0,1,0))));
  CGE::Matrix origintranslation(Matrix::Translation, center*-1);
  CGE::Matrix valuescale(Matrix::Scale, Vec3f(scale,scale,scale));
  CGE::Matrix ytozero(Matrix::Identity); ytozero.at(5) = 0;
  CGE::Matrix offset(Matrix::Translation, Vec3f(0,0.05,0));
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
  mStreets.init(Vec3d(),(mMaxBox-mMinBox)/2.0+Vec3d(1,1,1), Vec3f(10,0,10));
  //mTree.init(Vec3d(),(mMaxBox-mMinBox)/2.0+Vec3d(1,1,1));
  for (iter = mNodes.begin(); iter != mNodes.end(); ++iter){
    (*iter).second->mPos = mTransform*(*iter).second->mPos;
    //mTree.insert((*iter).second->mPos, (*iter).second);
  }
  //mTree.buildDebugVertexBuffer();
}

void MapChunk::renderOctreeCallback(MapChunk::Node* node){
  ++drawCount;
  //no geometry generated, so do it now
  if (node->mModel == NULL){
    GeoGen::addJob(mMap,node);
  }
  else{
    CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
    rend->setColor(0.2,0.2,0.2,1.0);
    node->mModel->setupMaterial();
    node->mModel->render();
    node->mModel->resetMaterial();
  }
}

CGE::OctreeStatic<SimpleMesh*>::TraversalState MapChunk::renderCB(const std::vector<SimpleMesh*>& values, const CGE::BBox& box, uint8 flags, void* data){
  CGE::Camera* cam = (CGE::Camera*)data;
  OctreeStatic<SimpleMesh*>::TraversalState state = OctreeStatic<SimpleMesh*>::RECURSE_TO_END;

  //check if we do not pass through
  if (!(flags & OctreeStatic<SimpleMesh*>::PASS_THROUGH)){
    Frustum::Result res = cam->getFrustum().checkBox(box);
    if (res == Frustum::OUTSIDE)
      return OctreeStatic<SimpleMesh*>::STOP_RECURSION;
    if (res == Frustum::INTERSECT)
      state = OctreeStatic<SimpleMesh*>::RECURSE_FURTHER;
  }

  for (uint32 i = 0; i < values.size(); ++i){
    values[i]->render();
    ++drawCount;
  }

  return state;
}

