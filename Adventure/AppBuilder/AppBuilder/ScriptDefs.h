#ifndef SCRIPTDEFS_H
#define SCRIPTDEFS_H

#include <list>
#include <vector>
#include <string>

class CCode;
class Object2D;
class ExecutionContext;

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
  EVT_LEVEL=EVT_RELEASE+1, //EVT_LEVEL HAS TO BE LAST
};

class CodeSegment{
public:
  CodeSegment(){mRefCount = new int(1);}
  CodeSegment(const CodeSegment& cs){
    mCodes = cs.mCodes;
    mRefCount = cs.mRefCount;
    ++(*mRefCount);
  }
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
protected:
  std::vector<CCode*> mCodes;
  int* mRefCount;
  std::vector<ExecutionContext*> mEmbeddedContexts;
};

class StackData{
  friend std::ostream& operator<<(std::ostream& strm, const StackData& data);
  friend std::istream& operator>>(std::istream& strm, StackData& data);
public:
  StackData() : mType(I), mStr(""), mInt(0) {}
  StackData(std::string str) : mType(S), mInt(0) {mStr = str; if (mStr == "true"){mBool = true; mType = B;}}
  StackData(int value) : mType(I) {mInt = value;}
  StackData(bool value) : mType(B) {mBool = value;}
  StackData(float value) : mType(F) {mFloat = value;}
  StackData(ExecutionContext* ctx) : mType(EC) {mContext = ctx;}
  std::string getString() const {return mStr;}
  int getInt() const {return mInt;}
  bool getBool() const {return mBool;}
  float getFloat() const {if (mType == I)return (float)mInt; return mFloat;}
  ExecutionContext* getEC() const {return mContext;}
  bool isInt() const {return mType == I;}
  bool isString() const {return mType == S;}
protected:
  enum Type{
    S, I, B, F, EC
  };
  Type mType;
  std::string mStr;
  union{
    int mInt;
    bool mBool;
    float mFloat;
    ExecutionContext* mContext;
  };
};

std::ostream& operator<<(std::ostream& strm, const StackData& data);
std::istream& operator>>(std::istream& strm, StackData& data);

class Stack{
public:
  Stack(){}
  Stack(const Stack& st){mVariables = st.mVariables;}
  ~Stack(){}
  void push(const StackData& v) {mVariables.push_back(v);}
  StackData pop() {
    StackData data = mVariables.back();
    mVariables.pop_back();
    return data;
  }
  void clear(){
    mVariables.clear();
  }
protected:
  std::vector<StackData> mVariables;
};

class ExecutionContext;
typedef void (*StepEndHandler) (ExecutionContext& ctx);

class ExecutionContext{
  friend class PcdkScript;
  friend class ScriptFunctions;
public:
  ExecutionContext(CodeSegment* segment, bool isGameObject, const std::string& objectinfo);
  ExecutionContext(const ExecutionContext& ctx);
  ~ExecutionContext();
  std::list<EngineEvent>& getEvents() {return mEvents;}
  void setEvent(EngineEvent evt);
  void setEvents(std::list<EngineEvent>& events);
  void resetEvent(EngineEvent evt);
  void resetEvents(bool leaveCurrentUntouched);
  bool isEventSet(EngineEvent evt);
  bool isRunning();
  EngineEvent getCommandEvent();
  Stack& stack() {return mStack;}
  void suspend(int time) {mSuspended = true; mSleepTime = time;}
  void resume();
  void reset(bool clearEvents, bool clearStack);
  void setOwner(Object2D* owner) {mOwner = owner;}
  void setExecuteOnce() {mExecuteOnce = true;}
  void setSkip() {mSkip = true; mSuspended = false;}
  void setIdle(bool idle) {mIdle = idle;}
  bool isSkipping() {return mSkip;}
  void setEventHandled() {mEventHandled = true;}
  bool isEventHandled() {return mEventHandled;}
protected:
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
};


#endif
