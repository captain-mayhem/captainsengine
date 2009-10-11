#ifndef CIL_H
#define CIL_H

#include <string>
#include <vector>

#include "ScriptDefs.h"

/*
CADD, CMUL, CSUB, CDIV, CLABEL, CBEQ, CBNE,
CBLT, CBGT, CBLE, CBGE, CBRA, CI2R, CR2I,
CRET, CASSGN, CLOAD, CSTORE, CPUSH, CCALL,
CVAR
*/

class Stack;

typedef int (*ScriptFunc) (ExecutionContext&, unsigned numArgs);

class CCode{
public:
  CCode(){}
  virtual ~CCode(){}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc)=0;
};

class CPUSH : public CCode{
public:
  CPUSH(const std::string& s) : mData(s) {}
  CPUSH(const int i) : mData(i) {}
  virtual ~CPUSH() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc){
    ctx.stack().push(mData);
    return ++pc;
  }
protected:
  StackData mData;
};

class CCALL : public CCode{
public:
  CCALL(ScriptFunc func, unsigned numArgs) : mFunc(func), mNumArgs(numArgs) {}
  virtual ~CCALL(){}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc){
    (*mFunc)(ctx, mNumArgs);
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
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc){
    return pc+mOffset;
  }
protected:
  int mOffset;
};

class CBNEEVT : public CBRA{
public:
  CBNEEVT(EngineEvent event) : mEvent(event) {}
  virtual ~CBNEEVT() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc);
protected:
  EngineEvent mEvent;
};

class CBEZERO : public CBRA{
public:
  CBEZERO() {}
  virtual ~CBEZERO() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc);
};

class CBNEZERO : public CBRA{
public:
  CBNEZERO() {}
  virtual ~CBNEZERO() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc);
};

#endif
