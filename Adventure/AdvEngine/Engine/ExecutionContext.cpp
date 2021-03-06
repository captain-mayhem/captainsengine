#include "ExecutionContext.h"
extern "C"{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
#include "Engine.h"
#include <system/allocation.h>

using namespace adv;

TR_CHANNEL_LVL(ADV_ExectionContext, TRACE_INFO);

CodeSegment::CodeSegment(const CodeSegment& cs){
  mCodes = cs.mCodes;
  mRefCount = cs.mRefCount;
  ++(*mRefCount);
}

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
  out << mEmbeddedContexts.size() << std::endl;
  for (unsigned i = 0; i < mEmbeddedContexts.size(); ++i){
    mEmbeddedContexts[i]->save(out);
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
  in >> size;
  mEmbeddedContexts.resize(size);
  for (unsigned i = 0; i < size; ++i){
    mEmbeddedContexts[i] = new ExecutionContext(in);
  }
}

ExecutionContext::ExecutionContext(CodeSegment* segment, bool isGameObject, const std::string& objectinfo) : 
mCode(segment), mIsGameObject(isGameObject), mObjectInfo(objectinfo),
mStack(), mPC(0), mSuspended(false), mSleepTime(0), mOwner(NULL), mSkip(false), mIdle(false), mEventHandled(false), mRefCount(1),
mSuspender(NULL), mShouldFinish(false), mLuaRet(LUA_OK), mLoop1(NULL){
  mL = newThread();
  mStack.setState(mL);
}

ExecutionContext::ExecutionContext(const ExecutionContext& ctx) : mLuaRet(LUA_OK){
  mL = newThread();
  if (ctx.mCode)
    mCode = new CodeSegment(*ctx.mCode);
  else{
    mCode = NULL;
    lua_State* L = ((ExecutionContext&)ctx).getState();
    lua_pushthread(L);
    lua_gettable(L, LUA_REGISTRYINDEX);
    lua_getfield(L, -1, "script");
    lua_xmove(L, mL, 1);
    lua_pop(L, 1);
    Engine::instance()->getInterpreter()->initLuaContext(this);
  }
  mIsGameObject = ctx.mIsGameObject;
  mObjectInfo = ctx.mObjectInfo;
  mStack = ctx.mStack;
  mStack.setState(mL);
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
  mShouldFinish = ctx.mShouldFinish;
  mLoop1 = ctx.mLoop1;
  if (mLoop1)
    mLoop1->ref();
}

const char* ExecutionContext::luaReader(lua_State* L, void* data, size_t* size){
  std::istream& in = *(std::istream*)data;
  size_t sz;
  in >> sz;
  char dummy;
  in.get(dummy);
  *size = sz;
  static std::vector<char> dat;
  if (sz == 0){
    return NULL;
  }
  dat.resize(sz);
  in.read(&dat[0], sz);
  return &dat[0];
}

ExecutionContext::ExecutionContext(std::istream& in) : 
mStack(), mPC(0), mSuspended(false), mSleepTime(0), mOwner(NULL), mSkip(false), mIdle(false), 
mEventHandled(false), mRefCount(1), mSuspender(NULL), mShouldFinish(false), mLuaRet(LUA_OK)
{
  mL = newThread();
  mStack.setState(mL);
  in >> mIsGameObject >> mObjectInfo;
  if (mObjectInfo == "none")
    mObjectInfo = "";
  in >> mSleepTime >> mSuspended;
  if (Engine::instance()->getSettings()->script_lang == PCDK_SCRIPT){
    mCode = new CodeSegment();
    mCode->load(in);
  }
  else{
    mCode = NULL;
    lua_load(mL, luaReader, &in, "saved chunk", "bt");
    Engine::instance()->getInterpreter()->initLuaContext(this);
  }
  std::string tag;
  in >> tag;
  if (tag == "loop1")
    mLoop1 = new ExecutionContext(in);
  else
    mLoop1 = NULL;
}

ExecutionContext::ExecutionContext(lua_State* L) :
mCode(NULL), mIsGameObject(false), mObjectInfo(""),
mStack(), mPC(0), mSuspended(false), mSleepTime(0), mOwner(NULL), mSkip(false), mIdle(false), mEventHandled(false), mRefCount(1),
mSuspender(NULL), mShouldFinish(false), mLuaRet(LUA_OK), mLoop1(NULL){
  mL = newThread();
  mStack.setState(mL);
  lua_xmove(L, mL, 1);
  Engine::instance()->getInterpreter()->initLuaContext(this);
}

ExecutionContext::~ExecutionContext(){
  delete mCode;
  if (mLoop1)
    mLoop1->unref();
  lua_pushthread(mL);
  lua_pushnil(mL);
  lua_settable(mL, LUA_REGISTRYINDEX);
}

void ExecutionContext::setEvent(EngineEvent evt){
  TR_USE(ADV_ExectionContext);
  if (mShouldFinish) //do not add new events
    return;
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
  if (mEvents.empty())
    mEvents.push_back(EVT_NONE); //insert dummy event so that the new event is not popped to early when added while script was somewhere
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

bool ExecutionContext::containsEvent(EngineEvent evt){
  for (std::list<EngineEvent>::iterator iter = mEvents.begin(); iter != mEvents.end(); ++iter){
    if (*iter == evt)
      return true;
  }
  return false;
}

bool ExecutionContext::isRunning(){
  bool running = (((mCode && mPC > 0) || mLuaRet != LUA_OK) && !mIdle) || !mEvents.empty();
  if (!running){
    running = mLoop1 ? mLoop1->isRunning() : false;
  }
  return running;
}

EngineEvent ExecutionContext::getCommandEvent(){
  for (std::list<EngineEvent>::iterator iter = mEvents.begin(); iter != mEvents.end(); ++iter){
    EngineEvent curr = *iter;
    if (curr >= EVT_USER_BEGIN && curr <= EVT_USER_MIRROR_END)
      return curr;
    if (curr == EVT_LINK || curr == EVT_GIVE_LINK)
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

int ExecutionContext::luaWriter(lua_State* L, const void* p, size_t sz, void* ud){
  std::ostream& out = *(std::ostream*)ud;
  out << " " << sz << " ";
  out.write((const char*)p, sz);
  return 0;
}

void ExecutionContext::save(std::ostream& out){
  out << mIsGameObject << " " << (mObjectInfo.empty() ? "none" : mObjectInfo) << " ";
  out << mSleepTime << " " << mSuspended << " ";
  if (mCode)
    mCode->save(out);
  else{
    lua_pushthread(mL);
    lua_gettable(mL, LUA_REGISTRYINDEX);
    lua_getfield(mL, -1, "script");
    lua_dump(mL, luaWriter, &out);
    lua_pop(mL, 2);
  }
  out << " ";
  if (mLoop1 == NULL)
    out << "none\n";
  else{
    out << "loop1\n";
    mLoop1->save(out);
  }
  out << "\n";
}

bool ExecutionContext::isLoop1(){
  CCode* code = mCode->get(0);
  if (code == NULL)
    return false;
  if (code->getType() != CCode::BNEEVT)
    return false;
  CBNEEVT* evt = (CBNEEVT*)code;
  return evt->getEvent() == EVT_LOOP1;
}

String ExecutionContext::resolveCharName(const String& name){
  TR_USE(ADV_ExectionContext);
  if (name == "self"){
    if (!mSelf.empty())
      return mSelf;
    CharacterObject* co = Engine::instance()->getCharacter("self");
    if (!co){
      return "";
    }
    return co->getName();
  }
  return name;
}

CharacterObject* ExecutionContext::getCharacter(const String& name){
  String realname = resolveCharName(name);
  if (realname.empty())
    return NULL;
  return Engine::instance()->getCharacter(realname);
}

lua_State* ExecutionContext::newThread(){
  lua_State* ret = Engine::instance()->getInterpreter()->allocNewState(this);
  return ret;
}
