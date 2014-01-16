#ifndef CGE_ACTIVEOBJECT_H
#define CGE_ACTIVEOBJECT_H

#include <system/thread.h>

namespace CGE{

class ActiveObject{
public:
  ActiveObject();
  virtual ~ActiveObject();
  void start();
  void stop();
protected:
  virtual bool run() = 0;
  Thread mThread;
  Mutex mMutex;
  Condition mCond;
private:
  static void threadFunc(void* data);
  bool mShouldStop;
};

}

#endif
