
#include "AtomicCounter.h"

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
}

void AtomicCounter::decrement(){
#ifdef WIN32
  InterlockedDecrement(&mNumber);
#endif
}

int32 AtomicCounter::getNumber(){
  return mNumber;
}

bool AtomicCounter::isZero(){
  return mNumber == 0;
}
