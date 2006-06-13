#ifndef THREAD_H
#define THREAD_H

namespace System {

class Thread{
public:
  int create(void (*proc)(void* data), void* data);
  void destroy();
private:
  int threadID_;
};

}

#endif
