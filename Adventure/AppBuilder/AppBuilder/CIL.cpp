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

unsigned CBNEROW::execute(ExecutionContext& ctx, unsigned pc){
  std::map<int,bool>::iterator iter = Engine::instance()->getInterpreter()->tsActive().find(mRow);
  if (iter == Engine::instance()->getInterpreter()->tsActive().end()){
    Engine::instance()->getInterpreter()->tsActive()[mRow] = mVisible;
    iter = Engine::instance()->getInterpreter()->tsActive().find(mRow);
  }
  if (!iter->second) //the text is invisible, so skip further processing
    return pc+mOffset;
  Vec2i extent = Engine::instance()->getFontRenderer()->getTextExtent(mText, 1);
  Engine::instance()->getInterpreter()->tsPos().y -= extent.y;
  Vec2i butsize(Engine::instance()->getInterpreter()->getTSWidth(), extent.y);
  ButtonObject* but = new ButtonObject(Engine::instance()->getInterpreter()->tsPos(), butsize, mText, mRow);
  Engine::instance()->addUIElement(but);
  int chosenRow = Engine::instance()->getInterpreter()->getVariables()["#button"].getInt();
  if (chosenRow == mRow){
    Engine::instance()->getInterpreter()->getVariables()["#button"] = 0;
    return ++pc;
  }
  return pc+mOffset;
}

unsigned CBE::execute(ExecutionContext& ctx, unsigned pc){
  int second = ctx.stack().pop().getInt();
  int first = ctx.stack().pop().getInt();
  if (first == second)
    return pc+mOffset;
  return ++pc;
}

unsigned CBNE::execute(ExecutionContext& ctx, unsigned pc){
  int second = ctx.stack().pop().getInt();
  int first = ctx.stack().pop().getInt();
  if (first != second)
    return pc+mOffset;
  return ++pc;
}

unsigned CBLT::execute(ExecutionContext& ctx, unsigned pc){
  int second = ctx.stack().pop().getInt();
  int first = ctx.stack().pop().getInt();
  if (first < second)
    return pc+mOffset;
  return ++pc;
}

unsigned CBLE::execute(ExecutionContext& ctx, unsigned pc){
  int second = ctx.stack().pop().getInt();
  int first = ctx.stack().pop().getInt();
  if (first <= second)
    return pc+mOffset;
  return ++pc;
}

unsigned CBGT::execute(ExecutionContext& ctx, unsigned pc){
  int second = ctx.stack().pop().getInt();
  int first = ctx.stack().pop().getInt();
  if (first > second)
    return pc+mOffset;
  return ++pc;
}

unsigned CBGE::execute(ExecutionContext& ctx, unsigned pc){
  int second = ctx.stack().pop().getInt();
  int first = ctx.stack().pop().getInt();
  if (first >= second)
    return pc+mOffset;
  return ++pc;
}

unsigned CLOAD::execute(ExecutionContext& ctx, unsigned pc){
  StackData dat = Engine::instance()->getInterpreter()->getVariables()[mVariable];
  ctx.stack().push(dat);
  return ++pc;
}

unsigned CTIMER::execute(ExecutionContext& ctx, unsigned pc){
  return ++pc;
}
