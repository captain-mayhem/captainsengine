#ifndef CIL_H
#define CIL_H

#include <string>
#include <vector>

#include "ScriptDefs.h"
#include "ExecutionContext.h"

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
  enum Type{
    PUSH,
    CALL,
    BRA,
    BNEEVT,
    BNEROW,
    BE,
    BNE,
    BLT,
    BLE,
    BGT,
    BGE,
    LOAD,
    ADD,
    SUB,
    MUL,
    DIV,
    CONCAT,
    CDTIMER,
    STATE,
  };
  CCode(){}
  virtual ~CCode(){}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc)=0;
  virtual Type getType()=0;
};

class CPUSH : public CCode{
public:
  CPUSH(const std::string& s) : mData(s) {}
  CPUSH(const int i) : mData(i) {}
  CPUSH(const float f) : mData(f) {}
  CPUSH(const ExecutionContext* ec) : mData((ExecutionContext*)ec) {}
  virtual ~CPUSH() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc){
    ctx.stack().push(mData);
    return ++pc;
  }
  virtual Type getType(){return PUSH;}
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
  virtual Type getType(){return CALL;}
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
  virtual Type getType(){return BRA;}
protected:
  int mOffset;
};

class CBNEEVT : public CBRA{
public:
  CBNEEVT(EngineEvent event) : mEvent(event) {}
  virtual ~CBNEEVT() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc);
  virtual Type getType(){return BNEEVT;}
protected:
  EngineEvent mEvent;
};

class CBNEROW : public CBRA{
public:
  CBNEROW(int row, const std::string& text, bool visible) : mRow(row), mText(text), mVisible(visible) {}
  virtual ~CBNEROW() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc);
  virtual Type getType(){return BNEROW;}
protected:
  int mRow;
  std::string mText;
  bool mVisible;
};

class CBE : public CBRA{
public:
  CBE() {}
  virtual ~CBE() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc);
  virtual Type getType(){return BE;}
};

class CBNE : public CBRA{
public:
  CBNE() {}
  virtual ~CBNE() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc);
  virtual Type getType(){return BNE;}
};

class CBLT : public CBRA{
public:
  CBLT() {}
  virtual ~CBLT() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc);
  virtual Type getType(){return BLT;}
};

class CBLE : public CBRA{
public:
  CBLE() {}
  virtual ~CBLE() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc);
  virtual Type getType(){return BLE;}
};

class CBGT : public CBRA{
public:
  CBGT() {}
  virtual ~CBGT() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc);
  virtual Type getType(){return BGT;}
};

class CBGE : public CBRA{
public:
  CBGE() {}
  virtual ~CBGE() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc);
  virtual Type getType(){return BGE;}
};

class CLOAD : public CCode{
public:
  CLOAD(const std::string& s) : mVariable(s) {}
  virtual ~CLOAD() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc);
  virtual Type getType(){return LOAD;}
  void changeVariable(const std::string& var) {mVariable += var;}
  const std::string& getVarname() {return mVariable;}
protected:
  std::string mVariable;
};

class CADD : public CCode{
public:
  CADD() {}
  virtual ~CADD() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc){
    int d2 = ctx.stack().pop().getInt();
    int d1 = ctx.stack().pop().getInt();
    ctx.stack().push(d1+d2);
    return ++pc;
  }
  virtual Type getType(){return ADD;}
};

class CSUB : public CCode{
public:
  CSUB() {}
  virtual ~CSUB() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc){
    int d2 = ctx.stack().pop().getInt();
    int d1 = ctx.stack().pop().getInt();
    ctx.stack().push(d1-d2);
    return ++pc;
  }
  virtual Type getType(){return SUB;}
};

class CMUL : public CCode{
public:
  CMUL() {}
  virtual ~CMUL() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc){
    int d2 = ctx.stack().pop().getInt();
    int d1 = ctx.stack().pop().getInt();
    ctx.stack().push(d1*d2);
    return ++pc;
  }
  virtual Type getType(){return MUL;}
};

class CDIV : public CCode{
public:
  CDIV() {}
  virtual ~CDIV() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc){
    int d2 = ctx.stack().pop().getInt();
    int d1 = ctx.stack().pop().getInt();
    ctx.stack().push(d1/d2);
    return ++pc;
  }
  virtual Type getType(){return DIV;}
};

class CCONCAT : public CCode{
public:
  CCONCAT() {}
  virtual ~CCONCAT() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc);
  virtual Type getType(){return CONCAT;}
};

class CTIMER : public CCode{
public:
  CTIMER(int time, ExecutionContext* ctx){
    mCommands = ctx;
    mCommands->suspend(time);
  }
  ~CTIMER(){
    mCommands->unref();
  }
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc);
  virtual Type getType(){return CDTIMER;}
protected:
  ExecutionContext* mCommands;
};

class CSTATE : public CCode{
public:
  enum State{
    NORMAL,
    IDLE,
  };
  CSTATE(State state) {mState = state;}
  virtual ~CSTATE() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc);
  virtual Type getType(){return STATE;}
protected:
  State mState;
};

#endif
