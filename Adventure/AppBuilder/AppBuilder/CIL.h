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

namespace adv{

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
    DECSHIFT,
    I2R,
  };
  CCode(){}
  virtual ~CCode(){}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc)=0;
  virtual Type getType()=0;
  virtual void save(std::ostream& out);
  static CCode* load(std::istream& in);
};

class CPUSH : public CCode{
public:
  CPUSH(const std::string& s) : mData(s) {}
  CPUSH(const int i) : mData(i) {}
  CPUSH(const float f) : mData(f) {}
  CPUSH(const ExecutionContext* ec) : mData((ExecutionContext*)ec) {}
  CPUSH(std::istream& in);
  virtual ~CPUSH() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc){
    ctx.stack().push(mData);
    return ++pc;
  }
  virtual Type getType(){return PUSH;}
  virtual void save(std::ostream& out);
protected:
  StackData mData;
};

class CCALL : public CCode{
public:
  CCALL(ScriptFunc func, const std::string& funcname, unsigned numArgs) : mFunc(func), mName(funcname), mNumArgs(numArgs) {}
  CCALL(std::istream& in);
  virtual ~CCALL(){}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc){
    (*mFunc)(ctx, mNumArgs);
    return ++pc;
  }
  virtual Type getType(){return CALL;}
  virtual void save(std::ostream& out);
protected:
  ScriptFunc mFunc;
  std::string mName;
  unsigned mNumArgs;
};

class CBRA : public CCode{
public:
  CBRA() : mOffset(1) {}
  CBRA(std::istream& in);
  virtual ~CBRA() {}
  virtual void setOffset(int offset) {mOffset = offset;}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc){
    return pc+mOffset;
  }
  virtual Type getType(){return BRA;}
  virtual void save(std::ostream& out);
protected:
  int mOffset;
};

class CBNEEVT : public CBRA{
public:
  CBNEEVT(EngineEvent event) : mEvent(event) {}
  CBNEEVT(std::istream& in);
  virtual ~CBNEEVT() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc);
  virtual Type getType(){return BNEEVT;}
  virtual void save(std::ostream& out);
  EngineEvent getEvent() {return mEvent;}
protected:
  EngineEvent mEvent;
};

class CBNEROW : public CBRA{
public:
  CBNEROW(int row, const std::string& text, bool visible) : mRow(row), mText(text), mVisible(visible) {}
  CBNEROW(std::istream& in);
  virtual ~CBNEROW() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc);
  virtual Type getType(){return BNEROW;}
  virtual void save(std::ostream& out);
protected:
  int mRow;
  std::string mText;
  bool mVisible;
};

class CBE : public CBRA{
public:
  CBE() {}
  CBE(std::istream& in) : CBRA(in){}
  virtual ~CBE() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc);
  virtual Type getType(){return BE;}
};

class CBNE : public CBRA{
public:
  CBNE() {}
  CBNE(std::istream& in) : CBRA(in){}
  virtual ~CBNE() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc);
  virtual Type getType(){return BNE;}
};

class CBLT : public CBRA{
public:
  CBLT() {}
  CBLT(std::istream& in) : CBRA(in){}
  virtual ~CBLT() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc);
  virtual Type getType(){return BLT;}
};

class CBLE : public CBRA{
public:
  CBLE() {}
  CBLE(std::istream& in) : CBRA(in){}
  virtual ~CBLE() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc);
  virtual Type getType(){return BLE;}
};

class CBGT : public CBRA{
public:
  CBGT() {}
  CBGT(std::istream& in) : CBRA(in){}
  virtual ~CBGT() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc);
  virtual Type getType(){return BGT;}
};

class CBGE : public CBRA{
public:
  CBGE() {}
  CBGE(std::istream& in) : CBRA(in){}
  virtual ~CBGE() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc);
  virtual Type getType(){return BGE;}
};

class CLOAD : public CCode{
public:
  CLOAD(const std::string& s) : mVariable(s) {}
  CLOAD(std::istream& in);
  virtual ~CLOAD() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc);
  virtual Type getType(){return LOAD;}
  void changeVariable(const std::string& var) {mVariable += var;}
  const std::string& getVarname() {return mVariable;}
  virtual void save(std::ostream& out);
protected:
  std::string mVariable;
};

class CADD : public CCode{
public:
  CADD() {}
  virtual ~CADD() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc){
    float d2 = ctx.stack().pop().getFloat();
    float d1 = ctx.stack().pop().getFloat();
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
    float d2 = ctx.stack().pop().getFloat();
    float d1 = ctx.stack().pop().getFloat();
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
    float d2 = ctx.stack().pop().getFloat();
    float d1 = ctx.stack().pop().getFloat();
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
    float d2 = ctx.stack().pop().getFloat();
    float d1 = ctx.stack().pop().getFloat();
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
  CTIMER(ExecutionContext* ctx){
    mCommands = ctx;
  }
  CTIMER(std::istream& in);
  ~CTIMER(){
    mCommands->unref();
  }
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc);
  virtual Type getType(){return CDTIMER;}
  virtual void save(std::ostream& out);
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
  CSTATE(std::istream& in);
  virtual ~CSTATE() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc);
  virtual Type getType(){return STATE;}
  virtual void save(std::ostream& out);
protected:
  State mState;
};

class CDecShift : public CCode{
public:
  CDecShift() {}
  virtual ~CDecShift() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc){
    int val = ctx.stack().pop().getInt();
    float shifted = (float)val;
    while (shifted > 1.0f)
      shifted *= 0.1f;
    ctx.stack().push(shifted);
    return ++pc;
  }
  virtual Type getType(){return DECSHIFT;}
};

class CI2R : public CCode{
public:
  CI2R() {}
  virtual ~CI2R() {}
  virtual unsigned execute(ExecutionContext& ctx, unsigned pc){
    int val = ctx.stack().pop().getInt();
    ctx.stack().push((float)val);
    return ++pc;
  }
  virtual Type getType(){return I2R;}
};


}

#endif
