
#include "Terrain.h"

#include "TerrainChunk.h"

using namespace CGE;

Terrain::Terrain(int16 width, int16 depth, float widthscale, float depthscale, uint8* heightdata, float heightscale) :
  mWidth(width),mDepth(depth), mScale(widthscale,heightscale,depthscale){
  mHeightfield = new uint8[width*depth];
  memset(mHeightfield, 0, width*depth*sizeof(uint8));
  mChunks.setTraversalCB(renderCB);
}

Terrain::~Terrain(){
  //for (int i = 0; i < mNumChunksY; ++i){
  //  delete [] mChunks[i];
  //}
  //delete [] mChunks;
  delete [] mHeightfield;
}

void Terrain::generateRandomHeights(){
  for (int i = 0; i < mWidth*mDepth; ++i){
    mHeightfield[i] = (uint8)((float)rand()/RAND_MAX*100);
  }
}

void Terrain::generateTerrainChunks(int chunksize){
  //determine dimensions of octree
  mChunks.init(Vec3f(), Vec3f(mWidth, 0, mDepth)*mScale, Vec3f(chunksize,chunksize,chunksize)*mScale);

  mNumChunksX = mWidth/chunksize;
  mNumChunksY = mDepth/chunksize;
  //mChunks = new TerrainChunk*[mNumChunksY];
  float y = -mDepth*mScale.z/2;
  for (int j = 0; j < mNumChunksY; ++j){
    //mChunks[j] = new TerrainChunk[mNumChunksX];
    float x = -mWidth*mScale.x/2;
    for (int i = 0; i < mNumChunksX; ++i){
      TerrainChunk* chunk = new TerrainChunk();
      chunk->generate(chunksize+1,mScale.x,x,y);
      mChunks.insert(chunk->getBox(), chunk);
      x += chunksize*mScale.x;
    }
    y += chunksize*mScale.z;
  }
  init(mWidth,mDepth,mScale.x,mScale.z,mHeightfield,mScale.y);
}

static int count;
void Terrain::render(CGE::Renderer& rend, CGE::Camera& cam){
  count = 0;
  mChunks.traverse(&cam);
  char buf[256];
  sprintf(buf, "Terrain chunks rendered: %i", count);
  CGE::Font* fnt = CGE::Engine::instance()->getFont(0);
  fnt->print(20,720,buf,0);
}

CGE::OctreeStatic<TerrainChunk*>::TraversalState Terrain::renderCB(const std::vector<TerrainChunk*>& values, const CGE::BBox& box, uint8 flags, void* data){
  CGE::Camera* cam = (CGE::Camera*)data;
  OctreeStatic<TerrainChunk*>::TraversalState state = OctreeStatic<TerrainChunk*>::RECURSE_TO_END;

  //check if we do not pass through
  if (!(flags & OctreeStatic<TerrainChunk*>::PASS_THROUGH)){
    Frustum::Result res = cam->getFrustum().checkBox(box);
    if (res == Frustum::OUTSIDE)
      return OctreeStatic<TerrainChunk*>::STOP_RECURSION;
    if (res == Frustum::INTERSECT)
      state = OctreeStatic<TerrainChunk*>::RECURSE_FURTHER;
  }

  for (uint32 i = 0; i < values.size(); ++i){
    values[i]->render(*cam);
    ++count;
  }

  return state;
}
