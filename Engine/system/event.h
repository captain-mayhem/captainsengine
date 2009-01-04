
#ifndef CGE_EVENT_H
#define CGE_EVENT_H

#ifdef WIN32
#include <Windows.h>
#endif

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
};

}

#endif
