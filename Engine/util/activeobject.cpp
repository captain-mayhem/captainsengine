#include "activeobject.h"
#include "../io/Tracing.h"

using namespace CGE;

TR_CHANNEL_LVL(CGE_ActiveObject, TRACE_DEBUG);

ActiveObject::ActiveObject() : mShouldStop(false) {

}

ActiveObject::~ActiveObject(){

}

void ActiveObject::start(){
  mThread.create(threadFunc, this);
}

void ActiveObject::stop(){
  mMutex.lock();
  mShouldStop = true;
  mCond.signal();
  mMutex.unlock();
  mThread.join();
}

void ActiveObject::threadFunc(void* data){
  TR_USE(CGE_ActiveObject);
  TR_DEBUG("entered thread loop");
  ActiveObject* self = (ActiveObject*)data;
  while(!self->mShouldStop){
    bool ret = !self->run();
    self->mMutex.lock();
    self->mShouldStop = self->mShouldStop || ret;
    self->mMutex.unlock();
  }
  TR_DEBUG("left thread loop");
}
