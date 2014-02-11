#ifndef ADV_LOADER_H
#define ADV_LOADER_H

#include <queue>

#include <util/activeobject.h>

#include "Ids.h"
#include "Vector.h"

namespace adv{

class EventQueue;

class Event{
public:
  virtual ~Event() {}
  virtual Event* execute() = 0;
};

class EventQueue{
public:
  void addEvent(Event* evt) {mQueue.push(evt);}
  Event* getEvent() {if (mQueue.empty()) return NULL; Event* evt = mQueue.front(); return evt;}
  void popEvent() {mQueue.pop();}
  bool empty() {return mQueue.empty();}
  unsigned size() {return mQueue.size();}
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
  void beamCharacter(const std::string& name, ExecutionContext* reason, const std::string& room, const Vec2i& pos, LookDir dir);
  bool handleResultEvent();
  void waitUntilFinished();
protected:
  virtual bool run();
private:
  AdvDocument* mData;
  EventQueue mQReq;
  EventQueue mQRes;
  CGE::Mutex mResMutex;
  CGE::Condition mResCond;
  PcdkScript* mCompiler;
};

}

#endif
