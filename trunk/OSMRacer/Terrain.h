#ifndef TERRAIN_H
#define TERRAIN_H

#include <system/types.h>
#include <common/OctreeStatic.h>
#include <physics/CollisionHeightfield.h>

namespace CGE{
  class Renderer;
  class Camera;
}

class TerrainChunk;

class Terrain : public CGE::CollisionHeightfield{
public:
  Terrain(int16 width, int16 depth, float widthscale, float depthscale, uint8* heightdata, float heightscale);
  ~Terrain();
  void generateRandomHeights();
  void generateTerrainChunks(int chunksize);
  void render(CGE::Renderer& rend, CGE::Camera& cam);
  float getHeight(float x, float z);
protected:
  static CGE::OctreeStatic<TerrainChunk*>::TraversalState renderCB(const std::vector<TerrainChunk*>& values, const CGE::BBox& box, uint8 flags, void* data);
  int16 mDepth;
  int16 mWidth;
  uint8* mHeightfield;
  CGE::Vec3f mScale;
  int16 mNumChunksX;
  int16 mNumChunksY;
  CGE::OctreeStatic<TerrainChunk*> mChunks;
};

#endif
