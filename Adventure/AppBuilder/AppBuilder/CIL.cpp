#include "CIL.h"
#include "Engine.h"

unsigned CBNEEVT::execute(ExecutionContext& ctx, unsigned pc){
  //check if event is set
  if (ctx.isEventSet(mEvent)){
    ctx.resetEvent(mEvent);
    return ++pc;
  }
  return pc+mOffset;
}

unsigned CBEZERO::execute(ExecutionContext& ctx, unsigned pc){
  bool check = ctx.stack().pop().getBool();
  if (check == false)
    return pc+mOffset;
  return ++pc;
}

unsigned CBNEZERO::execute(ExecutionContext& ctx, unsigned pc){
  bool check = ctx.stack().pop().getBool();
  if (check != false)
    return pc+mOffset;
  return ++pc;
}
