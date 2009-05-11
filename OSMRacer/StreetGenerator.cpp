
#include "StreetGenerator.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <mesh/SimpleMesh.h>

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

using namespace CGE;

StreetGenerator::StreetGenerator(MapChunk* map) : mMap(map) {

}

StreetGenerator::~StreetGenerator(){

}

void StreetGenerator::buildStreets(CGE::OctreeStatic<CGE::SimpleMesh*>& streets){
  MapChunk::Iterator iter = mMap->getNodeIterator();
  while (iter.hasNext()){
    MapChunk::Node* currNode = iter.next();
    if (mIntersections.find(currNode) == mIntersections.end()){
      reorderGrah(currNode);
      calculateIntersections(currNode);
    }
  }
  iter = mMap->getNodeIterator();
  while (iter.hasNext()){
    MapChunk::Node* currNode = iter.next();
    generateStreetGeometry(currNode, streets);
  }
}

static int compare(const void* a, const void* b){
  std::pair<float,MapChunk::Node*>* pa = (std::pair<float,MapChunk::Node*>*)a;
  std::pair<float,MapChunk::Node*>* pb = (std::pair<float,MapChunk::Node*>*)b;
  if (pa < pb)
    return -1;
  if (pa > pb)
    return 1;
  return 0;
}

void StreetGenerator::reorderGrah(MapChunk::Node* node){
  if (node->succs_.empty())
    return;
  std::pair<float,MapChunk::Node*>* nodes = new std::pair<float,MapChunk::Node*>[node->succs_.size()];
  MapChunk::Node* first = (MapChunk::Node*)node->succs_[0];
  Vec3f v1 = Vec3f(first->mPos - node->mPos);
  v1.normalize();
  nodes[0] = std::pair<float,MapChunk::Node*>(0,first);
  //generate angles
  for (unsigned i = 1; i < node->succs_.size(); ++i){
    MapChunk::Node* second = (MapChunk::Node*)node->succs_[i];
    Vec3f v2 = Vec3f(second->mPos - node->mPos);
    v2.normalize();
    float angle = getAngle(v1,v2,mMap->getNormal());
    nodes[i] = std::pair<float,MapChunk::Node*>(angle,second);
  }
  qsort(nodes, node->succs_.size(), sizeof(std::pair<float,MapChunk::Node*>), compare);
  for (unsigned i = 1; i < node->succs_.size(); ++i){
    node->succs_[i] = nodes[i].second;
  }
}

void StreetGenerator::calculateIntersections(MapChunk::Node* node){
  mIntersections[node] = std::vector<Vec3f>();
  int valence = node->succs_.size();
  if (valence == 0){
    return;
  }
  Vec3f v1 = ((MapChunk::Node*)node->succs_[node->succs_.size()-1])->mPos - node->mPos;
  float thickness1 = 2.5;
  Vec3f s1 = v1.cross(mMap->getNormal()).normalized()*thickness1;
  Vec3f p1 = node->mPos-s1;
  if (valence == 1){
    Vec3f p2 = node->mPos+s1;
    mIntersections[node].push_back(p2);
    mIntersections[node].push_back(p1);
    return;
  }
  for (int i = 0; i < valence; ++i){
    Vec3f v2 = ((MapChunk::Node*)node->succs_[i])->mPos - node->mPos;
    float thickness2 = 2.5;
    Vec3f s2 = v2.cross(mMap->getNormal()).normalized()*thickness2;
    Vec3f p2 = node->mPos+s2;
    Vec3f result = intersectLine(p1, v1, p2, v2);
    mIntersections[node].push_back(result);
    v1 = v2;
    s1 = s2;
    p1 = node->mPos-s1;
  }
}

float StreetGenerator::getAngle(const Vec3f& v1, const Vec3f& v2, const Vec3f& normal){
  float angle = acos(v1.dot(v2));
  Vec3f axis = v1.cross(v2);
  if (axis.dot(normal) < 0){
    angle = (float)(2*M_PI-angle);
  }
  return angle;
}

Vec3f StreetGenerator::intersectLine(const Vec3f& p1, const Vec3f& dir1, const Vec3f& p2, const Vec3f& dir2){
  float my = ((p2.z - p1.z)*dir1.x + (p1.x - p2.x)*dir1.z)/(dir2.x*dir1.z-dir2.z*dir1.x);
  return p2+dir2*my;
}

void StreetGenerator::generateStreetGeometry(MapChunk::Node* node, CGE::OctreeStatic<CGE::SimpleMesh*>& streets){
  //generate street crossing geometry
  if (node->succs_.size() >= 3){
    BBox crossingbox;
    VertexBuffer* crossing = Engine::instance()->getRenderer()->createVertexBuffer();
    crossing->create(VB_POSITION | VB_NORMAL, node->succs_.size()+2);
    crossing->lockVertexPointer();
    crossing->setPosition(0, node->mPos);
    crossing->setNormal(0, Vec3f(0,1,0));
    crossingbox.addPoint(node->mPos);
    for (unsigned i = 0; i < mIntersections[node].size(); ++i){
      crossing->setPosition(i+1, mIntersections[node][i]);
      crossing->setNormal(i+1, Vec3f(0,1,0));
      crossingbox.addPoint(mIntersections[node][i]);
    }
    crossing->setPosition(node->succs_.size()+1, mIntersections[node][0]);
    crossing->setNormal(node->succs_.size()+1, Vec3f(0,1,0));
    crossing->unlockVertexPointer();
    SimpleMesh* crossingmesh = new SimpleMesh(crossing, NULL, VB_Trifan);
    Vec3f mx = crossingbox.getMax();
    Vec3f mn = crossingbox.getMin();
    crossingbox = BBox(mn-Vec3f(1,0.1,1), mx+Vec3f(1,0.1,1));
    streets.insert(crossingbox, crossingmesh);
  }

  //generate street geometry
  if (node->succs_.size() != mIntersections[node].size()){
    int i = 0;
    ++i;
  }
  for (unsigned i = 0; i < node->succs_.size(); ++i){
    MapChunk::Node* destination = (MapChunk::Node*)node->succs_[i];
    if (mGenerated[std::make_pair(node,destination)] == true)
      continue;
    unsigned j;
    for (j = 0; j < destination->succs_.size(); ++j){
      if (destination->succs_[j] == node)
        break;
    }
    Vec3f sr = mIntersections[node][i];
    Vec3f sl = mIntersections[node][(i+1)%mIntersections[node].size()];
    Vec3f er = mIntersections[destination][(j+1)%mIntersections[destination].size()];
    Vec3f el = mIntersections[destination][j];
    BBox box;
    box.addPoint(sr);
    box.addPoint(sl);
    box.addPoint(er);
    box.addPoint(el);
    VertexBuffer* vb = Engine::instance()->getRenderer()->createVertexBuffer();
    vb->create(VB_POSITION | VB_NORMAL, 4);
    vb->lockVertexPointer();
    vb->setPosition(0, el);
    vb->setNormal(0, Vec3f(0,1,0));
    vb->setPosition(1, sl);
    vb->setNormal(1, Vec3f(0,1,0));
    vb->setPosition(2, er);
    vb->setNormal(2, Vec3f(0,1,0));
    vb->setPosition(3, sr);
    vb->setNormal(3, Vec3f(0,1,0));
    vb->unlockVertexPointer();
    SimpleMesh* mesh = new SimpleMesh(vb, NULL, VB_Tristrip);
    Vec3f mx = box.getMax();
    Vec3f mn = box.getMin();
    box = BBox(mn-Vec3f(1,0.1,1), mx+Vec3f(1,0.1,1));
    streets.insert(box, mesh);
    mGenerated[std::make_pair(destination,node)] = true;
    mGenerated[std::make_pair(node,destination)] = true;
  }
  //TODO mechanism to destroy geometry
}
