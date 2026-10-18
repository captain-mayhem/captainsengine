#ifndef CGE_ATOMIC_COUNTER_H
#define CGE_ATOMIC_COUNTER_H

#include <cge_export.h>

#include "types.h"

namespace CGE{

class CGE_API AtomicCounter{
public:
  AtomicCounter(int32 number);
  ~AtomicCounter();
  void increment();
  void decrement();
  int32 getNumber();
  bool isZero();
protected:
#ifdef WIN32
  volatile LONG mNumber;
#endif
#ifdef UNIX
  volatile int mNumber;
#endif
};

}

#endif
