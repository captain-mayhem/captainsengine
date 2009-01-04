
#ifndef CGE_EVENT_H
#define CGE_EVENT_H

#ifdef WIN32
#include <Windows.h>
#endif
#ifdef UNIX
#include <pthread.h>
#endif

#include "thread.h"

namespace System{

class Event{
public:
  Event();
  ~Event();
  void wait();
  void signal();
protected:
#ifdef WIN32
  HANDLE mEvent;
#endif
#ifdef UNIX
  pthread_cond_t mCondition;
#endif
  Mutex mMutex;
};

}

#endif

