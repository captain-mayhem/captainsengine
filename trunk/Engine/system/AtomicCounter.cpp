
#include "AtomicCounter.h"

#if defined(UNIX) && defined(OLD_ATOMICITY)
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
#ifdef OLD_ATOMICITY
  __gnu_cxx::__atomic_add(&mNumber, 1);
#else
  __sync_fetch_and_add(&mNumber, 1);
#endif
#endif
}

void AtomicCounter::decrement(){
#ifdef WIN32
  InterlockedDecrement(&mNumber);
#endif
#ifdef UNIX
#ifdef OLD_ATOMICITY
  __gnu_cxx::__atomic_add(&mNumber, -1);
#else
  __sync_fetch_and_add(&mNumber, -1);
#endif
#endif
}

int32 AtomicCounter::getNumber(){
  return mNumber;
}

bool AtomicCounter::isZero(){
  return mNumber == 0;
}
