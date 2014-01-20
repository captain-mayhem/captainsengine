#ifndef ADV_LOADER_H
#define ADV_LOADER_H

#include <queue>

#include <util/activeobject.h>

#include "Ids.h"

namespace adv{

class EventQueue;

class Event{
public:
  virtual Event* execute() = 0;
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
class AdvDocument;
class PcdkScript;

class ResLoader : public CGE::ActiveObject{
public:
  ResLoader();
  ~ResLoader();
  void setData(AdvDocument* data);
  void loadRoom(std::string name, bool isSubRoom, ExecutionContext* loadreason, ScreenChange change, int fading, int depthoffset);
  bool handleResultEvent();
protected:
  virtual bool run();
private:
  AdvDocument* mData;
  EventQueue mQReq;
  EventQueue mQRes;
  PcdkScript* mCompiler;
};

}

#endif
