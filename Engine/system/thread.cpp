#include "thread.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#endif

using namespace System;

int Thread::create(void (*proc)(void* data), void* data){
#ifdef WIN32
  threadID_ = (int)_beginthread(proc, 8192, data);
#endif
#ifdef UNIX 
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 8192);
  pthread_create(&threadID_, &attr, proc, data);
#endif
  return threadID_;
}

void Thread::destroy(){
#ifdef WIN32
  TerminateThread(&threadID_, 0);
#endif
#ifdef UNIX 
  pthread_cancel(&threadID_);
#endif
  //_endthread();
}