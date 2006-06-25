#ifndef THREAD_H
#define THREAD_H

#ifdef UNIX
#include <pthread.h>
#endif

namespace System {
  
class Thread{
public:
  int create(void (*proc)(void* data), void* data);
  void destroy();
private:
  long threadID_;
};

class Mutex{
public:
  Mutex();
  ~Mutex();
  void lock();
  void unlock();
private:
  pthread_mutex_t mutex_;
};

}

#endif
