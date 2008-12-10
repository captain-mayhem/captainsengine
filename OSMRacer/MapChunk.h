#include "common/octree.h"

class MapChunk{
public:
  MapChunk();
  ~MapChunk();
  void addStreetNode(double x, double y, double z, int id);
protected:
  Octree<double,int> mTree;
};