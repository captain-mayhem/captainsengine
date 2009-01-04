
#include "event.h"

using namespace System;

Event::Event(){
  mEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
}

Event::~Event(){
  CloseHandle(mEvent);
}

void Event::wait(){
  WaitForSingleObject(mEvent, INFINITE);
}

void Event::signal(){
  SetEvent(mEvent);
}