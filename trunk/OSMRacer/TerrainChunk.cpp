
#include "TerrainChunk.h"

#include "system/engine.h"
#include "renderer/renderer.h"
#include "renderer/camera.h"

using namespace CGE;

TerrainChunk::TerrainChunk(){
}

TerrainChunk::~TerrainChunk(){
  delete mVB;
  for (int i = 0; i < 5; ++i){
    delete mInds[i];
  }
  delete [] mInds;
}

void TerrainChunk::generate(int32 numVertices, float widthScale, float startX, float startZ){
  CGE::Renderer* rend = CGE::Engine::instance()->getRenderer();
  mVB = rend->createVertexBuffer();
  int numLODs = (int)(log((double)(numVertices-1))/log(2.0))+1;
  mVB->create(VB_POSITION | VB_NORMAL, numVertices*numVertices);
  mInds = new CGE::IndexBuffer*[numLODs];
  
  //generate vertices 
  mVB->lockVertexPointer();
  for (int j = 0; j < numVertices; ++j){
    for (int i = 0; i < numVertices; ++i){
      Vec3f p(startX+i*widthScale,0,startZ+j*widthScale);
      mBox.addPoint(p);
      mVB->setPosition(j*numVertices+i,p);
      mVB->setNormal(j*numVertices+i,Vec3f(0,1,0));
    }
  }
  mVB->unlockVertexPointer();
  
  //generate indices
  int skipSize = 1;
  for (int bufferNum = 0; bufferNum < numLODs; ++bufferNum){
    int numQuads = (numVertices-1)/skipSize;
    mInds[bufferNum] = rend->createIndexBuffer(CGE::IndexBuffer::IB_USHORT,2*numQuads*(numQuads+1));
    short* indices = (short*)mInds[bufferNum]->lockIndexPointer();
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
    mInds[bufferNum]->unlockIndexPointer();
    skipSize *= 2;
  }
  /*indices[0] = 0;
  indices[1] = numVertices*(numVertices-1)-1+1;
  indices[2] = numVertices-1;
  indices[3] = numVertices*numVertices-1;*/
}

void TerrainChunk::render(const CGE::Camera& cam){
  mVB->activate();
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
  mVB->draw(CGE::VB_Tristrip, mInds[lodLevel]);
}

CGE::BBox TerrainChunk::getBox(){
  return mBox;
}