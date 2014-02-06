#ifndef THREAD_H
#define THREAD_H

#ifdef WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif
#ifdef UNIX
#include <pthread.h>
#include <semaphore.h>
#endif

namespace CGE {
  
class Thread{
public:
  enum Priority{
    EXTREMELY_LOW,
    VERY_LOW,
    LOW,
    NORMAL,
    HIGH,
    VERY_HIGH,
    EXTREMELY_HIGH
  };
  int create(void (*proc)(void* data), void* data);
  void createSelf();
  void destroy();
  void join();
  static void sleep(int milliseconds);
  void setPriority(Priority priority);
private:
#ifdef WIN32
  HANDLE thread_;
#else
  pthread_t thread_;
#endif
};

class Mutex{
public:
  Mutex(bool recursive=false);
  ~Mutex();
  void lock();
  void unlock();
#ifdef WIN32
  operator HANDLE() {return mutex_;}
#endif
#ifdef UNIX
  operator pthread_mutex_t*() {return &mutex_;}
#endif
private:
#ifdef WIN32
  HANDLE mutex_;
  bool recursive_;
#endif
#ifdef UNIX
  pthread_mutex_t mutex_;
#endif
};

class Condition{
public:
  Condition();
  ~Condition();
  void wait(Mutex& mutex);
  bool waitTimeout(Mutex& mutex, int milliseconds);
  void signal();
protected:
#ifdef WIN32
  HANDLE mCond;
  CRITICAL_SECTION mMuty;
  int mWaiters;
  int mRelease;
  int mGeneration;
#endif
#ifdef UNIX
  pthread_cond_t mCond;
#endif
};

class Semaphore{
public:
  Semaphore(int count);
  ~Semaphore();
  void inc();
  void dec();
protected:
#ifdef WIN32
  HANDLE mSem;
#endif
#ifdef UNIX
  sem_t mSem;
#endif
};

}

#endif
