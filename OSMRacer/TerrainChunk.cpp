
#include "TerrainChunk.h"

#include "system/engine.h"
#include "renderer/renderer.h"

const int SCALE = 10;

TerrainChunk::TerrainChunk(){
}

TerrainChunk::~TerrainChunk(){
  delete mVB;
}

void TerrainChunk::generate(int32 numVertices){
  mVB = System::Engine::instance()->getRenderer()->createVertexBuffer();
  int numLODs = (int)(log((double)(numVertices-1))/log(2.0))+1;
  mVB->create(VB_POSITION, numVertices*numVertices, numLODs);
  
  //generate vertices 
  mVB->lockVertexPointer();
  for (int j = 0; j < numVertices; ++j){
    for (int i = 0; i < numVertices; ++i){
      mVB->setPosition(j*numVertices+i,Vec3f(i*SCALE,0,j*SCALE));
    }
  }
  mVB->unlockVertexPointer();
  
  //generate indices
  int skipSize = 1;
  for (int bufferNum = 0; bufferNum < numLODs; ++bufferNum){
    int numQuads = (numVertices-1)/skipSize;
    mVB->createIndexBuffer(bufferNum,2*numQuads*(numQuads+1));
    short* indices = mVB->lockIndexPointer(bufferNum);
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
    mVB->unlockIndexPointer(bufferNum);
    skipSize *= 2;
  }
  /*indices[0] = 0;
  indices[1] = numVertices*(numVertices-1)-1+1;
  indices[2] = numVertices-1;
  indices[3] = numVertices*numVertices-1;*/
}

void TerrainChunk::render(const Graphics::Camera& cam){
  mVB->activate();
  static int i = 0;
  static int skip = 0;
  mVB->draw(Graphics::VB_Tristrip, i);
  if (++skip > 100){
    i = (i+1)%5;
    skip = 0;
  }
}