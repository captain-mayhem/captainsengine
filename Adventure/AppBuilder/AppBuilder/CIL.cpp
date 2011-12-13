#include "CIL.h"
#include "Engine.h"

unsigned CBNEEVT::execute(ExecutionContext& ctx, unsigned pc){
  //check if event is set
  if (ctx.isEventSet(mEvent)){
    ctx.setEventHandled();
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
  std::vector<Vec2i> breakinfo;
  Vec2i extent = Engine::instance()->getFontRenderer()->getTextExtent(mText, Engine::instance()->getFontID(), breakinfo);
  extent.y /= breakinfo.size();
  if (!Engine::instance()->getInterpreter()->isTSTopToBottom())
    Engine::instance()->getInterpreter()->tsPos().y -= extent.y;
  Vec2i butsize(Engine::instance()->getInterpreter()->getTSWidth(), extent.y);
  ButtonObject* but = new ButtonObject(Engine::instance()->getInterpreter()->tsPos(), butsize, mText, mRow);
  if (Engine::instance()->getInterpreter()->isTSTopToBottom())
    Engine::instance()->addUIElement(but, extent.y);
  else
    Engine::instance()->addUIElement(but, 0);
  int chosenRow = Engine::instance()->getInterpreter()->getVariable("#button").getInt();
  if (chosenRow == mRow){
    Engine::instance()->getInterpreter()->setVariable("#button", 0);
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
  StackData dat = Engine::instance()->getInterpreter()->getVariable(mVariable);
  ctx.stack().push(dat);
  return ++pc;
}

unsigned CTIMER::execute(ExecutionContext& ctx, unsigned pc){
  Engine::instance()->getInterpreter()->execute(new ExecutionContext(*mCommands), true);
  return ++pc;
}

unsigned CSTATE::execute(ExecutionContext& ctx, unsigned pc){
  switch(mState){
    case NORMAL:
      ctx.setIdle(false);
      break;
    case IDLE:
      ctx.setIdle(true);
      break;
  }
  return ++pc;
}

static std::string stackDataToStr(const StackData& sd){
  if (sd.isString())
    return sd.getString();
  else if (sd.isInt()){
    char tmp[32];
    sprintf(tmp, "%i", sd.getInt());
    return tmp;
  }
  else
    DebugBreak();
  return "";
}

unsigned CCONCAT::execute(ExecutionContext& ctx, unsigned pc){
  StackData s2 = ctx.stack().pop();
  StackData s1 = ctx.stack().pop();
  std::string d1, d2;
  d1 = stackDataToStr(s1);
  d2 = stackDataToStr(s2);
  ctx.stack().push(d1+" "+d2);
  return ++pc;
}



void CCode::save(std::ostream& out){
  Type t = getType();
  out << (int)t;
}

CCode* CCode::load(std::istream& in){
  Type t;
  int tmp;
  in >> tmp;
  t = (Type)tmp;
  switch(t){
    case PUSH:
      return new CPUSH(in);
      break;
    default:
      DebugBreak();
  }
  return NULL;
}

void CPUSH::save(std::ostream& out){
  CCode::save(out);
  out << " " << mData;
}

CPUSH::CPUSH(std::istream& in){
  in >> mData;
}
