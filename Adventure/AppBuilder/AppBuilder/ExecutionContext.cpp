#include "ExecutionContext.h"
#include "Engine.h"

using namespace adv;

TR_CHANNEL_LVL(ADV_ExectionContext, TRACE_INFO);

void CodeSegment::removeLast(){
  delete mCodes.back();
  mCodes.pop_back();
}

void CodeSegment::save(std::ostream& out){
  out << mCodes.size() << " ";
  for (unsigned i = 0; i < mCodes.size(); ++i){
    mCodes[i]->save(out);
    out << "\n";
  }
}

void CodeSegment::load(std::istream& in){
  unsigned size;
  in >> size;
  mCodes.reserve(size);
  for (unsigned i = 0; i < size; ++i){
    CCode* code = CCode::load(in);
    mCodes.push_back(code);
  }
}

ExecutionContext::ExecutionContext(CodeSegment* segment, bool isGameObject, const std::string& objectinfo) : 
mCode(segment), mIsGameObject(isGameObject), mObjectInfo(objectinfo),
mStack(), mPC(0), mSuspended(false), mSleepTime(0), mOwner(NULL), mSkip(false), mIdle(false), mEventHandled(false), mRefCount(1),
mSuspender(NULL){

}

ExecutionContext::ExecutionContext(const ExecutionContext& ctx){
  mCode = new CodeSegment(*ctx.mCode);
  mIsGameObject = ctx.mIsGameObject;
  mObjectInfo = ctx.mObjectInfo;
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
  mSuspender = ctx.mSuspender;
}

ExecutionContext::ExecutionContext(std::istream& in) : 
mStack(), mPC(0), mSuspended(false), mSleepTime(0), mOwner(NULL), mSkip(false), mIdle(false), 
mEventHandled(false), mRefCount(1), mSuspender(NULL)
{
  in >> mIsGameObject >> mObjectInfo;
  if (mObjectInfo == "none")
    mObjectInfo = "";
  in >> mSleepTime >> mSuspended;
  mCode = new CodeSegment();
  mCode->load(in);
}

ExecutionContext::~ExecutionContext(){
  delete mCode;
}

void ExecutionContext::setEvent(EngineEvent evt){
  TR_USE(ADV_ExectionContext);
  if (evt == EVT_LOOP1 || evt == EVT_LOOP2){ //don't add loop events multiple times
    if (mSkip) //don't add loop events when the script is skipping
      return;
    for (std::list<EngineEvent>::iterator iter = mEvents.begin(); iter != mEvents.end(); ++iter){
      if (*iter == evt)
        return;
    }
  }
  if (mEvents.size() > 20)
    TR_WARN("Event %i added even though %i events remained unhandled", evt, mEvents.size());
  //if (!mSuspended)
  mEvents.push_back(evt);
}

void ExecutionContext::setEvents(std::list<EngineEvent>& events){
  mEvents = events;
}

void ExecutionContext::resetEvent(EngineEvent evt){
  mEvents.remove(evt);
  mEventHandled = false;
}

void ExecutionContext::resetNextEvent(){
  if (mEvents.size() < 2)
    return;
  std::list<EngineEvent>::iterator iter = mEvents.begin();
  ++iter;
  mEvents.erase(iter);
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
  return (mPC > 0 && !mIdle) || !mEvents.empty();
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
  if (mSuspender)
    mSuspender->forceResume();
  if (Engine::instance()->getInterpreter()->isBlockingScriptRunning() && mIdle) 
    reset(true, true);
}

void ExecutionContext::save(std::ostream& out){
  out << mIsGameObject << " " << (mObjectInfo.empty() ? "none" : mObjectInfo) << " ";
  out << mSleepTime << " " << mSuspended << " ";
  mCode->save(out);
}
