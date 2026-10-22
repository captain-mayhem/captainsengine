
#include "TerrainChunk.h"

#include "system/engine.h"
#include "renderer/camera.h"
#include "renderer/drawcontext.h"
#include "renderer/renderdevice.h"
#include "mesh/SimpleMesh.h"

using namespace CGE;

TerrainChunk::TerrainChunk(){
}

TerrainChunk::~TerrainChunk(){
}

void TerrainChunk::generate(int32 numVertices, float widthScale, float startX, float startZ){
  using namespace CGE::Gfx;
  RenderDevice* device = CGE::Engine::instance()->getRenderDevice();
  int numLODs = (int)(log((double)(numVertices-1))/log(2.0))+1;
  
  //generate vertices 
  std::vector<SimpleMesh::Vertex> vertices(numVertices*numVertices);
  for (int j = 0; j < numVertices; ++j){
    for (int i = 0; i < numVertices; ++i){
      Vec3f p(startX+i*widthScale,0,startZ+j*widthScale);
      mBox.addPoint(p);
      vertices[j*numVertices+i] = {{p.x, p.y, p.z}, {0, 1, 0}};
    }
  }
  mVB = device->createBuffer({BufferType::Vertex, Usage::Immutable, vertices.size()*sizeof(SimpleMesh::Vertex)}, vertices.data());
  
  //generate indices
  int skipSize = 1;
  for (int bufferNum = 0; bufferNum < numLODs; ++bufferNum){
    int numQuads = (numVertices-1)/skipSize;
    std::vector<uint16_t> indices(2*numQuads*(numQuads+1));
    int idx = 0;
    int curr_row = 0;
    for (int j = 0; j < (numVertices-1)/skipSize; ++j){
      int next_row = curr_row+numVertices*skipSize;
      indices[idx++] = curr_row;
      indices[idx++] = next_row;
      for (int i = 1; i <= (numVertices-1)/skipSize; ++i){
        indices[idx++] = curr_row+i*skipSize;
        indices[idx++] = next_row+i*skipSize;
      }
      curr_row = next_row;
    }
    mInds.push_back(device->createBuffer({BufferType::Index, Usage::Immutable, indices.size()*sizeof(uint16_t)}, indices.data()));
    mIndexCounts.push_back((int)indices.size());
    skipSize *= 2;
  }
  /*indices[0] = 0;
  indices[1] = numVertices*(numVertices-1)-1+1;
  indices[2] = numVertices-1;
  indices[3] = numVertices*numVertices-1;*/
}

void TerrainChunk::render(CGE::Gfx::DrawContext& ctx, const CGE::Camera& cam){
  if (!mVB)
    return;
  float dist = (cam.getPosition()-mBox.getCenter()).magnitude();
  float dist2 = dist*dist;
  int lodLevel;
  if (dist2 < 200)
    lodLevel = 0;
  else if (dist2 < 1000)
    lodLevel = 1;
  else if (dist2 < 2500)
    lodLevel = 2;
  else if (dist2 < 5000)
    lodLevel = 3;
  else
    lodLevel = 4;
  if (lodLevel >= (int)mInds.size())
    lodLevel = (int)mInds.size()-1;
  ctx.drawIndexed(CGE::Gfx::BuiltinShader::Lit, mVB.get(), SimpleMesh::getVertexLayout(), mInds[lodLevel].get(),
    CGE::Gfx::IndexType::UInt16, CGE::Gfx::PrimitiveType::TriangleStrip, mIndexCounts[lodLevel]);
}

CGE::BBox TerrainChunk::getBox(){
  return mBox;
}