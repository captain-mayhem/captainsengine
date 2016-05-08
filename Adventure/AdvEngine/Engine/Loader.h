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
  unsigned size() {return (unsigned)mQueue.size();}
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
  void loadRoom(std::string name, bool isSubRoom, ExecutionContext* loadreason, ScreenChange change, int fading, int depthoffset, bool executeEnter);
  void beamCharacter(const std::string& name, ExecutionContext* reason, const std::string& room, const Vec2i& pos, LookDir dir);
  void setFocus(const std::string& name, ExecutionContext* reason);
  void loadAnimation(const std::string& prefix, float fps, Vec2i pos, Vec2i size, ExecutionContext* loadreason, bool wait);
  bool handleResultEvent();
  void waitUntilFinished();
protected:
  virtual bool run();
private:
  AdvDocument* mData;
  EventQueue mQReq;
  EventQueue mQRes;
  CGE::Mutex mResMutex;
#ifndef ENGINE_SINGLE_THREADED
  CGE::Condition mResCond;
#endif
  PcdkScript* mCompiler;
};

}

#endif
