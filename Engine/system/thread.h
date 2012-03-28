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
#endif

namespace CGE {
  
class Thread{
public:
  int create(void (*proc)(void* data), void* data);
  void destroy();
  void join();
  static void sleep(int milliseconds);
private:
  long threadID_;
};

class Mutex{
public:
  Mutex();
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
#endif
#ifdef UNIX
  pthread_mutex_t mutex_;
#endif
};

}

#endif
