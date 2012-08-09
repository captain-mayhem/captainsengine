
#include "event.h"

using namespace CGE;

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

void Event::waitTimeout(int milliseconds){
#ifdef WIN32
  WaitForSingleObject(mEvent, milliseconds);
#endif
  mMutex.lock();
#ifdef UNIX
  struct timespec tv;
  clock_gettime(CLOCK_REALTIME, &tv);
  int seconds = milliseconds/1000;
  tv.tv_sec += seconds;
  tv_tv_nsec += (milliseconds-(seconds*1000))*1000000;
  pthread_cond_timedwait(&mEvent, mMutex, tv);
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

