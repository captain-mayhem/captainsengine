#include "common/octree.h"

class MapChunk{
public:
  MapChunk();
  ~MapChunk();
  void setDimensions(const Math::Vec3d& center, const Math::Vec3d& range);
  void addStreetNode(double x, double y, double z, int id);
  void render();
protected:
  Octree<double,int> mTree;
};