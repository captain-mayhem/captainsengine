#include "ExecutionContext.h"
#include "Engine.h"

void CodeSegment::removeLast(){
  delete mCodes.back();
  mCodes.pop_back();
}

ExecutionContext::ExecutionContext(CodeSegment* segment, bool isGameObject, const std::string& objectinfo) : 
mCode(segment), mIsGameObject(isGameObject), mObjectInfo(objectinfo),
mStack(), mPC(0), mSuspended(false), mSleepTime(0), mOwner(NULL), mSkip(false), mIdle(false), mEventHandled(false), mRefCount(1){

}

ExecutionContext::ExecutionContext(const ExecutionContext& ctx){
  mCode = new CodeSegment(*ctx.mCode);
  mIsGameObject = ctx.mIsGameObject;
  mObjectInfo = ctx.mIsGameObject;
  mStack = ctx.mStack;
  mPC = ctx.mPC;
  mEvents = ctx.mEvents;
  mExecuteOnce = ctx.mExecuteOnce;
  mSuspended = ctx.mSuspended;
  mSleepTime = ctx.mSleepTime;
  mOwner = ctx.mOwner;
  mSkip = ctx.mSkip;
  mIdle = ctx.mIdle;
  mEventHandled = ctx.mEventHandled;
  mRefCount = 1;
}

ExecutionContext::~ExecutionContext(){
  delete mCode;
}

void ExecutionContext::setEvent(EngineEvent evt){
  //if (!mSuspended)
  mEvents.push_back(evt);
}

void ExecutionContext::setEvents(std::list<EngineEvent>& events){
  mEvents = events;
}

void ExecutionContext::resetEvent(EngineEvent evt){
  if (mEvents.front() != evt)
    DebugBreak();
  mEvents.pop_front();
  mEventHandled = false;
}

void ExecutionContext::resetEvents(bool leaveCurrentUntouched){
  if (leaveCurrentUntouched){
    while(mEvents.size() > 1)
      mEvents.pop_back();
  }
  else{
    mEvents.clear();
    mEventHandled = false;
  }
}

bool ExecutionContext::isEventSet(EngineEvent evt){
  if (mEvents.empty())
    return false;
  return mEvents.front() == evt;
}

bool ExecutionContext::isRunning(){
  return mPC > 0 && !mIdle;
}

EngineEvent ExecutionContext::getCommandEvent(){
  for (std::list<EngineEvent>::iterator iter = mEvents.begin(); iter != mEvents.end(); ++iter){
    EngineEvent curr = *iter;
    if (curr >= EVT_USER_BEGIN && curr <= EVT_USER_MIRROR_END)
      return curr;
  }
  return EVT_NONE;
}

void ExecutionContext::reset(bool clearEvents, bool clearStack){
  if (clearStack)
    mStack.clear();
  if (clearEvents)
    mEvents.clear();
  mPC = 0;
  mSuspended = false;
  mSkip = false;
  mIdle = false;
  mEventHandled = false;
}

void ExecutionContext::resume(){
  mSuspended = false;
  if (Engine::instance()->getInterpreter()->isBlockingScriptRunning() && mIdle) 
    reset(true, true);
}
