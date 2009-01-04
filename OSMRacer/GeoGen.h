
#include <queue>
#include <system/event.h>
#include "MapChunk.h"

class GeoGen{
public:
  static void generateGeometry(void* data);
  static void addJob(MapChunk* map, MapChunk::Node* node);
protected:
  static System::Event mEvent;
  static System::Mutex mMutex;
  static std::queue<std::pair<MapChunk*,MapChunk::Node*> > mQ;

public:
  GeoGen(MapChunk* map, MapChunk::Node* node);
  ~GeoGen();
  void setMapChunk(MapChunk* map) {mCurMap = map;}
protected:
  void genStreets(MapChunk::Node* node);
  void genStreetSegment(Vec3f start, Vec3f end, Vec3f side);

  MapChunk* mCurMap;
  MeshGeo::Mesh* mMesh;
  int mLastIdx;
};

