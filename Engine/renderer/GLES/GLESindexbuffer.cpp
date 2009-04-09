
#include "GLESindexbuffer.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <GLES/gl.h>
#include <cassert>

using namespace CGE;

GLESIndexBuffer::GLESIndexBuffer(Type t, int32 size) : IndexBuffer(t,size){
  mIbSize = size;
  switch(t){
    case IB_UCHAR:
      mType = GL_UNSIGNED_BYTE;
      mInds = new uint8[mIbSize];
      break;
    case IB_USHORT:
      mType = GL_UNSIGNED_SHORT;
      mInds = (uint8*)new uint16[mIbSize];
      break;
    case IB_UINT:
      assert(false); //not possible with opengl es
  }
}

GLESIndexBuffer::~GLESIndexBuffer(){
  switch(mType){
    case GL_UNSIGNED_BYTE:
      delete [] mInds;
      break;
    case GL_UNSIGNED_SHORT:
      delete [] (uint16*)mInds;
      break;
  }
}

void* GLESIndexBuffer::lockIndexPointer(){
  return mInds;
}

void GLESIndexBuffer::unlockIndexPointer(){

}

void GLESIndexBuffer::setIndex(uint32 i, uint8 index){

}

void GLESIndexBuffer::setIndex(uint32 i, uint16 index){

}

void GLESIndexBuffer::setIndex(uint32 i, uint32 index){

}
