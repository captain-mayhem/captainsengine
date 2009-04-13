
#include "StreetGenerator.h"

#define _USE_MATH_DEFINES
#include <math.h>

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

using namespace CGE;

StreetGenerator::StreetGenerator(MapChunk* map) : mMap(map) {

}

StreetGenerator::~StreetGenerator(){

}

void StreetGenerator::buildStreets(){
  MapChunk::Iterator iter = mMap->getNodeIterator();
  while (iter.hasNext()){
    MapChunk::Node* currNode = iter.next();
    if (mIntersections.find(currNode) == mIntersections.end()){
      reorderGrah(currNode);
      calculateIntersections(currNode);
    }
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
  Vec3f v1 = first->mPos - node->mPos;
  v1.normalize();
  nodes[0] = std::pair<float,MapChunk::Node*>(0,first);
  //generate angles
  for (unsigned i = 1; i < node->succs_.size(); ++i){
    MapChunk::Node* second = (MapChunk::Node*)node->succs_[i];
    Vec3f v2 = second->mPos - node->mPos;
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
  if (valence <= 1){
    return;
  }
  Vec3f v1 = ((MapChunk::Node*)node->succs_[node->succs_.size()-1])->mPos - node->mPos;
  float thickness1 = 2.5;
  Vec3f s1 = v1.cross(mMap->getNormal()).normalized()*thickness1;
  Vec3f p1 = node->mPos-s1;
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
