
#include <queue>
#include "../system/event.h"
#include "../system/thread.h"

namespace CGE{

template <typename T>
class EventQueue{
public:
  EventQueue() {}

  ~EventQueue() {}

  void enqueue(T value) {
    mMutex.lock();
    mQ.push(value);
    mEvent.signal();
    mMutex.unlock();
  }

  void waitProcess(void (*process)(T)){
    mEvent.wait();
    tryProcess(process);
  }

  void tryProcess(void (*process)(T)){
    mMutex.lock();
    while(!mQ.empty()){
      T ret = mQ.front();
      mMutex.unlock();
      process(ret);
      mMutex.lock();
      mQ.pop();
    }
    mMutex.unlock();
  }
private:
  CGE::Event mEvent;
  CGE::Mutex mMutex;
  std::queue<T> mQ;
};

}
