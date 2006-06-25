#include "thread.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#endif
#ifdef UNIX
#include <pthread.h>
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
  pthread_create((pthread_t*)&threadID_, &attr, (void* (*)(void*))proc, data);
#endif
  return threadID_;
}

void Thread::destroy(){
#ifdef WIN32
  TerminateThread(&threadID_, 0);
#endif
#ifdef UNIX 
  pthread_cancel(threadID_);
#endif
  //_endthread();
}

Mutex::Mutex(){
#ifdef WIN32
  mutex_ = CreateMutex(0, FALSE, 0);
#endif
#ifdef UNIX
  pthread_mutex_init(&mutex_, NULL);
#endif
}

Mutex::~Mutex(){
#ifdef WIN32
  CloseHandle(mutex_);
#endif
#ifdef UNIX
  pthread_mutex_destroy(&mutex_);
#endif
}

void Mutex::lock(){
#ifdef WIN32
  WaitForSingleObject(mutex_, INFINITE);
#endif
#ifdef UNIX
  pthread_mutex_lock(&mutex_);
#endif
}

void Mutex::unlock(){
#ifdef WIN32
  ReleaseMutex(mutex_);
#endif
#ifdef UNIX
  pthread_mutex_unlock(&mutex_);
#endif
}
