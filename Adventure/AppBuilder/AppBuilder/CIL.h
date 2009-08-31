#ifndef CIL_H
#define CIL_H

/*
CADD, CMUL, CSUB, CDIV, CLABEL, CBEQ, CBNE,
CBLT, CBGT, CBLE, CBGE, CBRA, CI2R, CR2I,
CRET, CASSGN, CLOAD, CSTORE, CPUSH, CCALL,
CVAR
*/

enum EngineEvent{
  EVT_USER_BEGIN=0,
  EVT_USER_END=0x100,
  EVT_UNKNOWN=EVT_USER_END+1,
  EVT_MOUSE=EVT_UNKNOWN+1,
  EVT_CLICK=EVT_MOUSE+1,
};

class Stack;

typedef int (*ScriptFunc) (Stack&, unsigned numArgs);

class StackData{
public:
  StackData(std::string str) {mStr = str;}
  std::string getString() {return mStr;}
protected:
  std::string mStr;
};

class Stack{
public:
  void push(const StackData& v) {mVariables.push_back(v);}
  StackData pop() {
    StackData data = mVariables.back();
    mVariables.pop_back();
    return data;
  }
protected:
  std::vector<StackData> mVariables;
};

class CCode{
public:
  CCode(){}
  virtual ~CCode(){}
  virtual unsigned execute(Stack& stack, unsigned pc)=0;
};

class CPUSH : public CCode{
public:
  CPUSH(const std::string& s) : mData(s) {}
  virtual ~CPUSH() {}
  virtual unsigned execute(Stack& stack, unsigned pc){
    stack.push(mData);
    return ++pc;
  }
protected:
  StackData mData;
};

class CCALL : public CCode{
public:
  CCALL(ScriptFunc func, unsigned numArgs) : mFunc(func), mNumArgs(numArgs) {}
  virtual ~CCALL(){}
  virtual unsigned execute(Stack& stack, unsigned pc){
    (*mFunc)(stack, mNumArgs);
    return ++pc;
  }
protected:
  ScriptFunc mFunc;
  unsigned mNumArgs;
};

class CBRA : public CCode{
public:
  CBRA() : mOffset(1) {}
  virtual ~CBRA() {}
  virtual void setOffset(int offset) {mOffset = offset;}
  virtual unsigned execute(Stack& stack, unsigned pc){
    return pc+mOffset;
  }
protected:
  int mOffset;
};

class CBNEEVT : public CBRA{
public:
  CBNEEVT(EngineEvent event) : mEvent(event) {}
  virtual ~CBNEEVT() {}
  virtual unsigned execute(Stack& stack, unsigned pc){
    //check if event is set;
    if (false)
      return ++pc;
    return pc+mOffset;
  }
protected:
  EngineEvent mEvent;
};

#endif
