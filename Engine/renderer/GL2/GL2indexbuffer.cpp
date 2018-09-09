
#include "GL2indexbuffer.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#if !defined QNX && !defined ANDROID
#include <GL/gl.h>
#else
#include <GLES2/gl2.h>
#endif

using namespace CGE;

GL2IndexBuffer::GL2IndexBuffer(Type t, int32 size) : IndexBuffer(t,size){
  mIbSize = size;
  switch(t){
    case IB_UCHAR:
      mGLType = GL_UNSIGNED_BYTE;
      mInds = new uint8[mIbSize];
      break;
    case IB_USHORT:
      mGLType = GL_UNSIGNED_SHORT;
      mInds = (uint8*)new uint16[mIbSize];
      break;
    case IB_UINT:
      mGLType = GL_UNSIGNED_INT;
      mInds = (uint8*)new uint32[mIbSize];
      break;
  }
}

GL2IndexBuffer::~GL2IndexBuffer(){
  switch(mGLType){
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
  setIndex(i, (uint32)index);
}

void GL2IndexBuffer::setIndex(uint32 i, uint16 index){
  setIndex(i, (uint32)index);
}

void GL2IndexBuffer::setIndex(uint32 i, uint32 index){
  switch (mType){
  case IB_UCHAR:
  {
    uint8* pos = mInds + i*sizeof(uint8);
    *pos = index;
    break;
  }
  case IB_USHORT:
  {
    uint16* pos = (uint16*)(mInds + i*sizeof(uint16));
    *pos = index;
    break;
  }
  case IB_UINT:
  {
    uint32* pos = (uint32*)(mInds + i*sizeof(uint32));
    *pos = index;
    break;
  }
  }
}