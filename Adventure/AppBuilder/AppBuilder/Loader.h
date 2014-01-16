#ifndef ADV_LOADER_H
#define ADV_LOADER_H

#include <queue>

#include <util/activeobject.h>

namespace adv{

class Event{

};

class EventQueue{
private:
  std::queue<Event*> mQueue;
};

/*class ExecutionContext;

struct LoadRoom{
  std::string roomName;
  ExecutionContext* reason;
  ScreenChange screenchange;
};*/

class ResLoader : public CGE::ActiveObject{
public:
  void loadRoom();
protected:
  virtual bool run();
private:
  EventQueue mQ;
};

}

#endif
