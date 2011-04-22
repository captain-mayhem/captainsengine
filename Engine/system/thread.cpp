#include "thread.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#ifndef UNDER_CE
#include <process.h>
#endif
#endif
#ifdef UNIX
#include <pthread.h>
#endif
#include <stdlib.h>
#include <system/engine.h>

using namespace CGE;

int Thread::create(void (*proc)(void* data), void* data){
#ifdef WIN32
  //threadID_ = (int)_beginthread(proc, 8192, data);
  threadID_ = (int)CreateThread(NULL, 8192, (LPTHREAD_START_ROUTINE)proc, data, 0, NULL);
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
#ifndef NO_PTHREAD_CANCEL
  pthread_cancel(threadID_);
#else
	CGE::Log << "pthread_cancel not supported by this platform";
	abort();
#endif
#endif
  //_endthread();
}

void Thread::join(){
#ifdef WIN32
  WaitForSingleObject((HANDLE)threadID_, INFINITE);
#endif
#ifdef UNIX
  pthread_join(threadID_, NULL);
#endif
}

void Thread::sleep(int milliseconds){
#ifdef WIN32
  Sleep(milliseconds);
#endif
#ifdef UNIX
  struct timespec tv;
  tv.tv_sec = milliseconds/1000;
  tv.tv_nsec = (milliseconds-tv.tv_sec*1000)*1000000;
  nanosleep(&tv, NULL);
#endif
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
