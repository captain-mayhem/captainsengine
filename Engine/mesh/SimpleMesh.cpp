
#include "SimpleMesh.h"

#include "../renderer/vertexbuffer.h"

using namespace CGE;

SimpleMesh::SimpleMesh(VertexBuffer* vb, IndexBuffer* ib, PrimitiveType rt){
  mVB = vb;
  mIB = ib;
  mRenderType = rt;
}

SimpleMesh::~SimpleMesh(){
  
}

void SimpleMesh::render(){
  mVB->activate();
  mVB->draw(mRenderType, mIB);
}