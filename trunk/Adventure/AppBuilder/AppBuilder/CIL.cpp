#include "CIL.h"
#include "Engine.h"
#include <system/allocation.h>

using namespace adv;

TR_CHANNEL(ADV_CIL);

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
  extent.y /= (int)breakinfo.size();
  if (!Engine::instance()->getInterpreter()->isTSTopToBottom())
    Engine::instance()->getInterpreter()->tsPos().y -= extent.y;
  Vec2i butsize(Engine::instance()->getInterpreter()->getTSWidth(), extent.y);
  ButtonObject* but = new ButtonObject(Engine::instance()->getInterpreter()->tsPos(), butsize, mText, mRow);
  if (Engine::instance()->getInterpreter()->isTSTopToBottom())
    Engine::instance()->addUIElement(but, extent.y);
  else
    Engine::instance()->addUIElement(but, 0);
  int chosenRow = Engine::instance()->getInterpreter()->getVariable("!button").getInt();
  if (chosenRow == mRow){
    Engine::instance()->getInterpreter()->setVariable("!button", 0);
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

unsigned CSLOAD::execute(ExecutionContext& ctx, unsigned pc){
  String variable = ctx.stack().pop().getString();
  StackData dat = Engine::instance()->getInterpreter()->getVariable(variable);
  ctx.stack().push(dat);
  return ++pc;
}

unsigned CTIMER::execute(ExecutionContext& ctx, unsigned pc){
  float time = ctx.stack().pop().getFloat();
  mCommands->suspend((int)(time*1000), NULL);
  Engine::instance()->getInterpreter()->addTimer(new ExecutionContext(*mCommands));
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

static String stackDataToStr(const StackData& sd){
  if (sd.isString())
    return sd.getString();
  else if (sd.isNumber()){
    if (sd.getFloat() == (float)sd.getInt()){
      char tmp[32];
      sprintf(tmp, "%i", sd.getInt());
      return tmp;
    }
    else{
      char tmp[32];
      sprintf(tmp, "%.2f", sd.getFloat());
      int pos = 0;
      while (tmp[pos] != '\0'){
        if (tmp[pos] == '.'){
          tmp[pos] = ',';
        }
        ++pos;
      }
      return tmp;
    }
  }
  else{
    TR_USE(ADV_CIL);
    TR_BREAK("Unsupported conversion");
  }
  return "";
}

static bool isDigit(char ch){
  return ch >= '0' && ch <= '9';
}

unsigned CCONCAT::execute(ExecutionContext& ctx, unsigned pc){
  StackData s2 = ctx.stack().pop();
  StackData s1 = ctx.stack().pop();
  String d1, d2;
  d1 = stackDataToStr(s1);
  d2 = stackDataToStr(s2);
  String space = " ";
  if (d2.size() > 0 && (d2[0] == '.' || d2[0] == ':' || d2[0] == ',' || d2[0] == ')' || d2[0] == '?'))
    space = "";
  if (d1.size() > 0 && d2.size() > 0){
    char lastch = d1[d1.size()-1];
    char prelastch = 'x';
    if (d1.size() > 1)
      prelastch = d1[d1.size()-2];
    if ((lastch == '.' || lastch == ':') && isDigit(d2[0]) && isDigit(prelastch))
      space = "";
    if (lastch == '(')
      space = "";
    if (lastch == '0' && isDigit(d2[0]))
      space = "";
  }
  ctx.stack().push(d1+space+d2);
  return ++pc;
}



void CCode::save(std::ostream& out){
  Type t = getType();
  out << (int)t;
}

CCode* CCode::load(std::istream& in){
  TR_USE(ADV_CIL);
  Type t;
  int tmp;
  in >> tmp;
  t = (Type)tmp;
  switch(t){
    case PUSH:
      return new CPUSH(in);
    case CALL:
      return new CCALL(in);
    case BRA:
      return new CBRA(in);
    case BNEEVT:
      return new CBNEEVT(in);
    case BNEROW:
      return new CBNEROW(in);
    case BE:
      return new CBE(in);
    case BNE:
      return new CBNE(in);
    case BLT:
      return new CBLT(in);
    case BLE:
      return new CBLE(in);
    case BGT:
      return new CBGT(in);
    case BGE:
      return new CBGE(in);
    case LOAD:
      return new CLOAD(in);
    case SLOAD:
      return new CSLOAD();
    case ADD:
      return new CADD();
    case SUB:
      return new CSUB();
    case MUL:
      return new CMUL();
    case DIV:
      return new CDIV();
    case INTDIV:
      return new CIDIV();
    case CONCAT:
      return new CCONCAT();
    case CDTIMER:
      return new CTIMER(in);
    case STATE:
      return new CSTATE(in);
    case DECSHIFT:
      return new CDecShift();
    case I2R:
      return new CI2R();
    default:
      TR_BREAK("Input stream corrupted?");
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

void CCALL::save(std::ostream& out){
  CCode::save(out);
  out << " " << mName << " " << mNumArgs;
}

CCALL::CCALL(std::istream& in){
  in >> mName >> mNumArgs;
  mFunc = Engine::instance()->getInterpreter()->getFunction(mName);
}

void CBRA::save(std::ostream& out){
  CCode::save(out);
  out << " " << mOffset;
}

CBRA::CBRA(std::istream& in){
  in >> mOffset;
}

void CBNEEVT::save(std::ostream& out){
  CBRA::save(out);
  out << " " << (int)mEvent;
}

CBNEEVT::CBNEEVT(std::istream& in) : CBRA(in){
  int tmp;
  in >> tmp;
  mEvent = (EngineEvent)tmp;
}

void CBNEROW::save(std::ostream& out){
  CBRA::save(out);
  StackData txt(mText);
  out << " " << mRow << " " << mVisible << " " << txt;
}

CBNEROW::CBNEROW(std::istream& in) : CBRA(in){
  in >> mRow >> mVisible;
  StackData tmp;
  in >> tmp;
  mText = tmp.getString();
}

void CLOAD::save(std::ostream& out){
  CCode::save(out);
  out << " " << mVariable;
}

CLOAD::CLOAD(std::istream& in){
  in >> mVariable;
}

void CTIMER::save(std::ostream& out){
  CCode::save(out);
  out << " ";
  mCommands->save(out);
}

CTIMER::CTIMER(std::istream& in){
  mCommands = new ExecutionContext(in);
}

void CSTATE::save(std::ostream& out){
  CCode::save(out);
  out << " " << (int)mState;
}

CSTATE::CSTATE(std::istream& in){
  int tmp;
  in >> tmp;
  mState = (State)tmp;
}

unsigned CBRK::execute(ExecutionContext& ctx, unsigned pc){
  TR_USE(ADV_CIL);
  TR_BREAK("Script breakpoint hit");
  if (mOrig)
    return mOrig->execute(ctx, pc);
  else
    return ++pc;
}

void CBRK::save(std::ostream& out){
  if (mOrig)
    mOrig->save(out);
}
