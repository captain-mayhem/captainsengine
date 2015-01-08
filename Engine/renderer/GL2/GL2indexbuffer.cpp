
#include "GL2indexbuffer.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <GL/gl.h>

using namespace CGE;

GL2IndexBuffer::GL2IndexBuffer(Type t, int32 size) : IndexBuffer(t,size){
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
      mType = GL_UNSIGNED_INT;
      mInds = (uint8*)new uint32[mIbSize];
      break;
  }
}

GL2IndexBuffer::~GL2IndexBuffer(){
  switch(mType){
    case GL_UNSIGNED_BYTE:
      delete [] mInds;
      break;
    case GL_UNSIGNED_SHORT:
      delete [] (uint16*)mInds;
      break;
    case GL_UNSIGNED_INT:
      delete [] (uint32*)mInds;
      break;
  }
}

void* GL2IndexBuffer::lockIndexPointer(){
  return mInds;
}

void GL2IndexBuffer::unlockIndexPointer(){

}

void GL2IndexBuffer::setIndex(uint32 i, uint8 index){

}

void GL2IndexBuffer::setIndex(uint32 i, uint16 index){

}

void GL2IndexBuffer::setIndex(uint32 i, uint32 index){

}