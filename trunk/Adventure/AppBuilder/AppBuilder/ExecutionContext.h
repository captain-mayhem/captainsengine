#ifndef EXECUTIONCONTEXT_H
#define EXECUTIONCONTEXT_H

#include <list>
#include <vector>
#include <string>
#include "ScriptDefs.h"

namespace adv{

class CCode;
class Object2D;
class ExecutionContext;
class CharacterObject;

enum EngineEvent{
  EVT_USER_RANGE=0x50,
  EVT_NONE=0,
  EVT_USER_BEGIN=EVT_NONE+1,
  EVT_USER_END=EVT_USER_BEGIN+EVT_USER_RANGE,
  EVT_USER_MIRROR_BEGIN=EVT_USER_END+1,
  EVT_USER_MIRROR_END=EVT_USER_MIRROR_BEGIN+EVT_USER_RANGE,
  EVT_RESERVED=EVT_USER_MIRROR_END+1,
  EVT_MOUSE=EVT_RESERVED+1,
  EVT_CLICK=EVT_MOUSE+1,
  EVT_LINK=EVT_CLICK+1,
  EVT_CANT_ALL=EVT_LINK+1,
  EVT_GIVE_LINK=EVT_CANT_ALL+1,
  EVT_ENTER=EVT_GIVE_LINK+1,
  EVT_EXIT=EVT_ENTER+1,
  EVT_LOOP1=EVT_EXIT+1,
  EVT_LOOP2=EVT_LOOP1+1,
  EVT_DBLCLCK=EVT_LOOP2+1,
  EVT_MOUSEOUT=EVT_DBLCLCK+1,
  EVT_RELEASE=EVT_MOUSEOUT+1,
  EVT_RIGHTCLICK=EVT_RELEASE+1,
  EVT_LEVEL=EVT_RIGHTCLICK+1, //EVT_LEVEL HAS TO BE LAST
};

class CodeSegment{
public:
  CodeSegment() : mLoop1(NULL) {mRefCount = new int(1);}
  CodeSegment(const CodeSegment& cs);
  ~CodeSegment();
  void addCode(CCode* code){
    mCodes.push_back(code);
  }
  void addEmbeddedContext(ExecutionContext* ctx){
    mEmbeddedContexts.push_back(ctx);
  }
  CCode* get(unsigned pc){
    if (this && pc < mCodes.size())
      return mCodes[pc];
    return NULL;
  }
  unsigned numInstructions(){
    return (unsigned)mCodes.size();
  }
  void removeLast();
  void save(std::ostream& out);
  void load(std::istream& in);
  void setLoop1(ExecutionContext* ctx) {mLoop1 = ctx;}
  ExecutionContext* getLoop1() {return mLoop1;}
protected:
  std::vector<CCode*> mCodes;
  int* mRefCount;
  std::vector<ExecutionContext*> mEmbeddedContexts;
  ExecutionContext* mLoop1;
};

class ExecutionContext;
typedef void (*StepEndHandler) (ExecutionContext& ctx);

class Suspender{
public:
  virtual void forceResume()=0;
};

class NoopSuspender : public Suspender{
  virtual void forceResume() {}
};

class ExecutionContext{
  friend class PcdkScript;
  friend class ScriptFunctions;
  friend class CCALL;
public:
  ExecutionContext(CodeSegment* segment, bool isGameObject, const std::string& objectinfo);
  ExecutionContext(const ExecutionContext& ctx);
  ExecutionContext(std::istream& in);
  std::list<EngineEvent>& getEvents() {return mEvents;}
  void setEvent(EngineEvent evt);
  void setEvents(std::list<EngineEvent>& events);
  void resetEvent(EngineEvent evt);
  void resetNextEvent();
  void resetEvents(bool leaveCurrentUntouched);
  bool isEventSet(EngineEvent evt);
  bool containsEvent(EngineEvent evt);
  bool isRunning();
  EngineEvent getCommandEvent();
  Stack& stack() {return mStack;}
  void suspend(int time, Suspender* suspender) {mSuspended = true; mSleepTime = time; mSuspender = suspender;}
  void resume();
  void reset(bool clearEvents, bool clearStack);
  void setOwner(Object2D* owner) {mOwner = owner;}
  void setExecuteOnce() {mExecuteOnce = true;}
  bool isExecuteOnce() {return mExecuteOnce;}
  void setSkip(bool skip=true) {mSkip = skip; mSuspended = false;}
  bool isSkipping() {return mSkip;}
  void setIdle(bool idle) {mIdle = idle;}
  bool isIdle() {return mIdle;}
  void setEventHandled() {mEventHandled = true;}
  bool isEventHandled() {return mEventHandled;}
  bool isSuspended() {return mSuspended;}
  void ref() {++mRefCount;}
  bool unref() {if (this == NULL) return false; --mRefCount; if (mRefCount == 0){ delete this; return true;} return false; }
  CodeSegment* getCode() {return mCode;}
  void save(std::ostream& out);
  ExecutionContext* getLoop1() {return mCode->getLoop1();}
  bool isGameObject() {return mIsGameObject;}
  void finish() {mShouldFinish = true; setSkip(); if (mCode->getLoop1() != NULL)mCode->getLoop1()->finish();}
  void cancelFinish() {mShouldFinish = false; setSkip(false); if (mCode->getLoop1() != NULL) mCode->getLoop1()->cancelFinish();}
  bool isLoop1();
  void setSelf(const String& name) {mSelf = name;}
  String resolveCharName(const String& name);
  CharacterObject* getCharacter(const String& name);
  void setObjectInfo(const std::string& info) {mObjectInfo = info;}
  void setUseObjectName(const String& link) {mLinkName = link;}
  void setGiveObjectName(const String& give) {mGiveLinkName = give;}
  String getUseObjectName() {return mLinkName;}
  String getGiveObjectName() {return mGiveLinkName;}
protected:
  ~ExecutionContext();
  CodeSegment* mCode;
  bool mIsGameObject;
  std::string mObjectInfo;
  Stack mStack;
  unsigned mPC;
  std::list<EngineEvent> mEvents;
  bool mExecuteOnce;
  bool mSuspended;
  int mSleepTime;
  Object2D* mOwner;
  bool mSkip;
  bool mIdle;
  bool mEventHandled;
  int mRefCount;
  Suspender* mSuspender;
  bool mShouldFinish;
  String mSelf;
  String mLinkName;
  String mGiveLinkName;
};

}

#endif
