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

TR_CHANNEL(CGE_Thread)

int Thread::create(void (*proc)(void* data), void* data){
  int threadID = 0;
#ifdef WIN32
  thread_ = CreateThread(NULL, 8192, (LPTHREAD_START_ROUTINE)proc, data, 0, (LPDWORD)&threadID);
#endif
#ifdef UNIX 
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 8192);
  pthread_create(&thread_, &attr, (void* (*)(void*))proc, data);
  threadID = (int)thread_;
#endif
  return threadID;
}

void Thread::createSelf(){
#ifdef WIN32
  thread_ = GetCurrentThread();
#else
  thread_ = pthread_self();
#endif
}

void Thread::destroy(){
  TR_USE(CGE_Thread);
#ifdef WIN32
  TerminateThread(&thread_, 0);
#endif
#ifdef UNIX 
#ifndef NO_PTHREAD_CANCEL
  pthread_cancel(thread_);
#else
	TR_ERROR("pthread_cancel not supported by this platform");
	abort();
#endif
#endif
  //_endthread();
}

void Thread::join(){
#ifdef WIN32
  WaitForSingleObject(thread_, INFINITE);
#endif
#ifdef UNIX
  pthread_join(thread_, NULL);
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

void Thread::setPriority(Priority priority){
#ifdef WIN32
  int winprio = 0;
  switch(priority){
    case EXTREMELY_HIGH:
      winprio = THREAD_PRIORITY_TIME_CRITICAL;
      break;
    case VERY_HIGH:
      winprio = THREAD_PRIORITY_HIGHEST;
      break;
    case HIGH:
      winprio = THREAD_PRIORITY_ABOVE_NORMAL;
      break;
    case NORMAL:
      winprio = THREAD_PRIORITY_NORMAL;
      break;
    case LOW:
      winprio = THREAD_PRIORITY_BELOW_NORMAL;
      break;
    case VERY_LOW:
      winprio = THREAD_PRIORITY_LOWEST;
      break;
    case EXTREMELY_LOW:
      winprio = THREAD_PRIORITY_IDLE;
      break;
  }
  SetThreadPriority(thread_, winprio);
#else
#ifndef NO_PTHREAD_CANCEL
  pthread_setschedprio(thread_, 10+((int)priority)-3);
#else
  TR_USE(CGE_Thread);
  TR_ERROR("pthread_setschedprio not supported by this platform");
#endif
#endif
}

Mutex::Mutex(bool recursive)
#ifdef WIN32
: recursive_(recursive)
#endif
{
#ifdef WIN32
  if (recursive)
    mutex_ = CreateMutex(0, FALSE, 0);
  else
    mutex_ = CreateSemaphore(NULL, 1, 1, NULL);
#endif
#ifdef UNIX
  if (!recursive)
    pthread_mutex_init(&mutex_, NULL);
  else{
    pthread_mutexattr_t mta;
    pthread_mutexattr_init(&mta);
    pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&mutex_, &mta);
    pthread_mutexattr_destroy(&mta);
  }
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
  if (recursive_)
    ReleaseMutex(mutex_);
  else
    ReleaseSemaphore(mutex_, 1, NULL);
#endif
#ifdef UNIX
  pthread_mutex_unlock(&mutex_);
#endif
}

Condition::Condition(){
#ifdef WIN32
  mCond = CreateEvent(NULL,FALSE,FALSE,NULL);
#endif
#ifdef UNIX
  pthread_cond_init(&mCond,NULL);
#endif
}

Condition::~Condition(){
#ifdef WIN32
  CloseHandle(mCond);
#endif
#ifdef UNIX
  pthread_cond_destroy(&mCond);
#endif
}

void Condition::wait(Mutex& mutex){
#ifdef WIN32
  mutex.unlock();
  WaitForSingleObject(mCond, INFINITE);
  mutex.lock();
#endif
#ifdef UNIX
  pthread_cond_wait(&mCond, mutex);
#endif
}

void Condition::waitTimeout(Mutex& mutex, int milliseconds){
#ifdef WIN32
  mutex.unlock();
  WaitForSingleObject(mCond, milliseconds);
  mutex.lock();
#endif
#ifdef UNIX
  struct timespec tv;
  clock_gettime(CLOCK_REALTIME, &tv);
  int seconds = milliseconds/1000;
  tv.tv_sec += seconds;
  tv.tv_nsec += (milliseconds-(seconds*1000))*1000000;
  pthread_cond_timedwait(&mCond, mutex, &tv);
#endif
}

void Condition::signal(){
#ifdef WIN32
  SetEvent(mCond);
#endif
#ifdef UNIX
  pthread_cond_signal(&mCond);
#endif
}
