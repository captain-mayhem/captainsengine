#include "activeobject.h"

using namespace CGE;

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
  ActiveObject* self = (ActiveObject*)data;
  while(!self->mShouldStop){
    bool ret = !self->run();
    self->mMutex.lock();
    self->mShouldStop = self->mShouldStop || ret;
    self->mMutex.unlock();
  }
}
