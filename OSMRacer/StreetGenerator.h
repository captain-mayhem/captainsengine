#ifndef STREETGENERATOR_H
#define STREETGENERATOR_H

#include "MapChunk.h"
#include <math/vector.h>

namespace CGE{
class SimpleMesh;
}

class StreetGenerator{
public:
  StreetGenerator(MapChunk* map);
  ~StreetGenerator();
  void buildStreets(CGE::OctreeStatic<CGE::SimpleMesh*>& streets);
protected:
  void reorderGrah(MapChunk::Node* node);
  void calculateIntersections(MapChunk::Node* node);
  void generateStreetGeometry(MapChunk::Node* node, CGE::OctreeStatic<CGE::SimpleMesh*>& streets);
  static float getAngle(const CGE::Vec3f& v1, const CGE::Vec3f& v2, const CGE::Vec3f& normal);
  static CGE::Vec3f intersectLine(const CGE::Vec3f& p1, const CGE::Vec3f& dir1, const CGE::Vec3f& p2, const CGE::Vec3f& dir2);
  MapChunk* mMap;
  std::map<MapChunk::Node*,std::vector<CGE::Vec3f>> mIntersections;
};

#endif