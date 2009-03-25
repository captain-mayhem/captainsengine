
#include <queue>
#include <system/event.h>
#include <io/eventqueue.h>
#include "MapChunk.h"

class GeoGen{
public:
  static void generateGeometry(void* data);
  static void process(std::pair<MapChunk*,MapChunk::Node*> val);
  static void useGeometry();
  static void processAnswer(std::pair<MapChunk::Node*,MeshGeo::Model*> val);
  static void addJob(MapChunk* map, MapChunk::Node* node);
protected:
  static CGE::EventQueue<std::pair<MapChunk*,MapChunk::Node*> > mQueue;
  static CGE::EventQueue<std::pair<MapChunk::Node*,MeshGeo::Model*> > mAnswer;

public:
  GeoGen(MapChunk* map, MapChunk::Node* node);
  ~GeoGen();
  void setMapChunk(MapChunk* map) {mCurMap = map;}
protected:
  void genStreets(MapChunk::Node* node);
  void genStreetSegment(CGE::Vec3f start, CGE::Vec3f end, CGE::Vec3f rightside, CGE::Vec3f leftside);
  void reorderGraph(MapChunk::Node* node);
  static float getAngle(const CGE::Vec3f& v1, const CGE::Vec3f& v2, const CGE::Vec3f& normal);

  MapChunk* mCurMap;
  MeshGeo::Mesh* mMesh;
  int mLastIdx;
};

