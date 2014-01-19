#ifndef ADV_LOADER_H
#define ADV_LOADER_H

#include <queue>

#include <util/activeobject.h>

#include "Ids.h"

namespace adv{

class Event{
public:
  virtual void execute() = 0;
};

class EventQueue{
public:
  void addEvent(Event* evt) {mQueue.push(evt);}
  Event* getEvent() {if (mQueue.empty()) return NULL; Event* evt = mQueue.front(); mQueue.pop(); return evt;}
  bool empty() {return mQueue.empty();}
private:
  std::queue<Event*> mQueue;
};

class ExecutionContext;

class ResLoader : public CGE::ActiveObject{
public:
  void loadRoom(std::string name, bool isSubRoom, ExecutionContext* loadreason, ScreenChange change, int fading);
protected:
  virtual bool run();
private:
  EventQueue mQ;
};

}

#endif
