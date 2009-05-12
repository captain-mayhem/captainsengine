
#include "AtomicCounter.h"

#ifdef UNIX
#include <bits/atomicity.h>
#endif

using namespace CGE;


AtomicCounter::AtomicCounter(int32 number){
  mNumber = number;
}

AtomicCounter::~AtomicCounter(){

}

void AtomicCounter::increment(){
#ifdef WIN32
  InterlockedIncrement(&mNumber);
#endif
#ifdef UNIX
  __gnu_cxx::__atomic_add(&mNumber, 1);
#endif
}

void AtomicCounter::decrement(){
#ifdef WIN32
  InterlockedDecrement(&mNumber);
#endif
#ifdef UNIX
  __gnu_cxx::__atomic_add(&mNumber, -1);
#endif
}

int32 AtomicCounter::getNumber(){
  return mNumber;
}

bool AtomicCounter::isZero(){
  return mNumber == 0;
}
