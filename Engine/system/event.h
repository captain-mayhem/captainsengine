
#ifndef CGE_EVENT_H
#define CGE_EVENT_H

#include <cge_export.h>

#ifdef WIN32
#include <Windows.h>
#endif
#ifdef UNIX
#include <pthread.h>
#endif

#include "thread.h"

namespace CGE{

class CGE_API Event{
public:
  Event();
  ~Event();
  void wait();
  void waitTimeout(int milliseconds);
  void signal();
  //void reset();
protected:
#ifdef WIN32
  HANDLE mEvent;
#endif
#ifdef UNIX
  pthread_cond_t mEvent;
  Mutex mMutex;
#endif
};

}

#endif

