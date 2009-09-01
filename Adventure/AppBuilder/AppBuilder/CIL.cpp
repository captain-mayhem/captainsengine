#include "CIL.h"
#include "Engine.h"

unsigned CBNEEVT::execute(Stack& stack, unsigned pc){
  //check if event is set
  if (Engine::instance()->getInterpreter()->isEventSet(mEvent))
    return ++pc;
  return pc+mOffset;
}
