
#include "event.h"

using namespace System;

Event::Event(){
#ifdef WIN32
  mEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
#endif
#ifdef UNIX
  pthread_cond_init(&mEvent,NULL);
#endif
}

Event::~Event(){
#ifdef WIN32
  CloseHandle(mEvent);
#endif
#ifdef UNIX
  pthread_cond_destroy(&mEvent);
#endif
}

void Event::wait(){
#ifdef WIN32
  WaitForSingleObject(mEvent, INFINITE);
#endif
  mMutex.lock();
#ifdef UNIX
  pthread_cond_wait(&mEvent, mMutex);
#endif
  reset();
}

void Event::signal(){
#ifdef WIN32
  SetEvent(mEvent);
#endif
#ifdef UNIX
  pthread_cond_signal(&mEvent);
#endif
}

void Event::reset(){
#ifdef WIN32
  ResetEvent(mEvent);
#endif
  mMutex.unlock();
#ifdef UNIX
#endif
}

