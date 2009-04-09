
#ifndef CGE_EVENT_H
#define CGE_EVENT_H

#ifdef WIN32
#include <Windows.h>
#endif
#ifdef UNIX
#include <pthread.h>
#endif

#include "thread.h"

namespace CGE{

class Event{
public:
  Event();
  ~Event();
  void wait();
  void signal();
  void reset();
protected:
#ifdef WIN32
  HANDLE mEvent;
#endif
#ifdef UNIX
  pthread_cond_t mEvent;
#endif
  Mutex mMutex;
};

}

#endif

