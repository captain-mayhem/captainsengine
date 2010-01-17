#ifndef SCRIPTDEFS_H
#define SCRIPTDEFS_H

#include <set>

class CCode;
class Object2D;

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
  EVT_LEVEL=EVT_LOOP2+1,
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
  std::string getString() {return mStr;}
  int getInt() {return mInt;}
  bool getBool() {return mBool;}
  float getFloat() {if (mType == I)return (float)mInt; return mFloat;}
protected:
  enum Type{
    S, I, B, F,
  };
  Type mType;
  std::string mStr;
  union{
    int mInt;
    bool mBool;
    float mFloat;
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
  void setEvent(EngineEvent evt);
  void resetEvent(EngineEvent evt);
  bool isEventSet(EngineEvent evt);
  EngineEvent getCommandEvent();
  Stack& stack() {return mStack;}
  void setStepEndHandler(StepEndHandler handler){
    mHandler = handler;
  }
  void suspend(int time) {mSuspended = true; mSleepTime = time;}
  void resume() {mSuspended = false;}
  void reset(bool clearEvents);
  void setOwner(Object2D* owner) {mOwner = owner;}
  void setExecuteOnce() {mExecuteOnce = true;}
protected:
  CodeSegment* mCode;
  bool mIsGameObject;
  std::string mObjectInfo;
  Stack mStack;
  unsigned mPC;
  std::set<EngineEvent> mEvents;
  bool mExecuteOnce;
  StepEndHandler mHandler;
  bool mSuspended;
  int mSleepTime;
  Object2D* mOwner;
};


#endif
