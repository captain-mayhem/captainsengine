#ifndef SCRIPTDEFS_H
#define SCRIPTDEFS_H

#include <set>

class CCode;

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
};

class CodeSegment{
public:
  CodeSegment(){}
  ~CodeSegment();
  void addCode(CCode* code){
    mCodes.push_back(code);
  }
  CCode* get(unsigned pc){
    if (pc < mCodes.size())
      return mCodes[pc];
    return NULL;
  }
  unsigned numInstructions(){
    return (unsigned)mCodes.size();
  }
protected:
  std::vector<CCode*> mCodes;
};

class StackData{
public:
  StackData(std::string str) : mInt(0) {mStr = str; if (mStr == "true"){mBool = true;}}
  StackData(int value) {mInt = value;}
  StackData(bool value) {mBool = value;}
  std::string getString() {return mStr;}
  int getInt() {return mInt;}
  bool getBool() {return mBool;}
protected:
  std::string mStr;
  union{
    int mInt;
    bool mBool;
  };
};

class Stack{
public:
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
typedef void (*StepEndHandler) (ExecutionContext& ctx, void* data);

class ExecutionContext{
  friend class PcdkScript;
public:
  ExecutionContext(CodeSegment* segment);
  ~ExecutionContext();
  void setEvent(EngineEvent evt);
  void resetEvent(EngineEvent evt);
  bool isEventSet(EngineEvent evt);
  EngineEvent getCommandEvent();
  Stack& stack() {return mStack;}
  void setStepEndHandler(StepEndHandler handler, void* data){
    mHandler = handler;
    mData = data;
  }
  void suspend() {mSuspended = true;}
  void resume() {mSuspended = false;}
  void reset();
protected:
  CodeSegment* mCode;
  Stack mStack;
  unsigned mPC;
  std::set<EngineEvent> mEvents;
  bool mExecuteOnce;
  StepEndHandler mHandler;
  void* mData;
  bool mSuspended;
};


#endif
