
#include "event.h"

using namespace System;

Event::Event(){
#ifdef WIN32
  mEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
#endif
#ifdef UNIX
#endif
}

Event::~Event(){
#ifdef WIN32
  CloseHandle(mEvent);
#endif
#ifdef UNIX
#endif
}

void Event::wait(){
#ifdef WIN32
  WaitForSingleObject(mEvent, INFINITE);
#endif
#ifdef UNIX
#endif
}

void Event::signal(){
#ifdef WIN32
  SetEvent(mEvent);
#endif
#ifdef UNIX
#endif
}

