#include "Script.h"

#include <cassert>
#include <string>
#include <wx/msgdlg.h>

#include "Engine.h"

CodeSegment::~CodeSegment(){
  for (unsigned i = 0; i < mCodes.size(); ++i){
    delete mCodes[i];
  }
}

PcdkScript::PcdkScript(AdvDocument* data) : mData(data) {
  registerFunction("loadroom", loadRoom);
  registerFunction("setfocus", setFocus);
  registerFunction("showinfo", showInfo);
  registerFunction("walkto", walkTo);
  registerFunction("speech", speech);
  registerFunction("pickup", pickup);
  registerFunction("playsound", playSound);
  registerFunction("if_bool", isBoolEqual);
  registerFunction("if_obj", isObjectInState);
  registerFunction("setlight", setLight);
  registerFunction("setbool", setBool);
  registerFunction("if_command", isCommandSet);
  registerFunction("setobj", setObj);
  registerFunction("beamto", beamTo);
  mBooleans = data->getProjectSettings()->booleans;
}

PcdkScript::~PcdkScript(){
  for (std::list<ExecutionContext*>::iterator iter = mScripts.begin(); iter != mScripts.end(); ++iter){
    (*iter)->reset();
    delete *iter;
  }
  mScripts.clear();
}

ExecutionContext* PcdkScript::parseProgram(std::string program){
  pANTLR3_INPUT_STREAM input;
  ppcdkLexer lexer;
  pANTLR3_COMMON_TOKEN_STREAM tokStream;
  ppcdkParser parser;
  pcdkParser_prog_return pcdkAST;
  input = antlr3NewAsciiStringInPlaceStream(((pANTLR3_UINT8)program.c_str()), (ANTLR3_UINT32)program.size(), (pANTLR3_UINT8)"Script");
  lexer = pcdkLexerNew(input);
  tokStream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lexer));
  parser = pcdkParserNew(tokStream);
  pcdkAST = parser->prog(parser);
  NodeList* p = pcdkAST.nodes;
  if (parser->pParser->rec->state->errorCount > 0){
    wxMessageBox("Error parsing script");
    return NULL;
  }
  parser->free(parser);
  tokStream->free(tokStream);
  lexer->free(lexer);
  input->free(input);
  CodeSegment* segment = new CodeSegment;
  transform(p, segment, START);
  delete p;
  if (segment->numInstructions() == 0){
    delete segment;
    return NULL;
  }
  return new ExecutionContext(segment);
}

unsigned PcdkScript::transform(NodeList* program, CodeSegment* codes, TrMode mode){
  unsigned count = 0;
  if (mode == ARGLIST){
    program->reset(false);
    while(program->hasPrev()){
      ASTNode* ast = program->prev();
      count += transform(ast, codes);
    }
  }
  else{
    program->reset(true);
    while(program->hasNext()){
      ASTNode* ast = program->next();
      count += transform(ast, codes);
    }
  }
  return count;
}

unsigned PcdkScript::transform(ASTNode* node, CodeSegment* codes){
  unsigned count = 0;
  switch(node->getType()){
      case ASTNode::FUNCCALL:{
        FuncCallNode* fc = static_cast<FuncCallNode*>(node);
        if (fc->getName() == "break"){
          CBRA* jmp = new CBRA();
          codes->addCode(jmp);
          ++count;
          mUnresolvedBranches.push_back(std::make_pair(jmp,codes->numInstructions()));
          break;
        }
        ScriptFunc f = mFunctions[fc->getName()];
        if (f == NULL){
          DebugBreak();
        }
        count += transform(fc->getArguments(), codes, ARGLIST);
        codes->addCode(new CCALL(f, fc->getArguments()->size()));
        ++count;
      }
      break;
      case ASTNode::IDENTIFIER:{
        IdentNode* id = static_cast<IdentNode*>(node);
        codes->addCode(new CPUSH(id->value()));
        ++count;
      }
      break;
      case ASTNode::INTEGER:{
        IntNode* number = static_cast<IntNode*>(node);
        codes->addCode(new CPUSH(number->value()));
        ++count;
      }
      break;
      case ASTNode::EVENT:{
        EventNode* evt = static_cast<EventNode*>(node);
        EngineEvent evtcode = getEngineEvent(evt->getEvent());
        CBNEEVT* cevt = new CBNEEVT(evtcode);
        codes->addCode(cevt);
        ++count;
        int offset = transform(evt->getBlock(), codes, START);
        cevt->setOffset(offset+1);
        count += offset;
        for (std::list<std::pair<CBRA*,unsigned> >::iterator iter = mUnresolvedBranches.begin(); iter != mUnresolvedBranches.end(); ++iter){
          unsigned diff = codes->numInstructions() - iter->second;
          iter->first->setOffset(diff+1);
        }
        mUnresolvedBranches.clear();
      }
      break;
      case ASTNode::CONDITION:{
        CondNode* cond = static_cast<CondNode*>(node);
        ScriptFunc f = mFunctions[cond->getCondFuncName()];
        if(f == NULL){
          DebugBreak();
        }
        count += transform(cond->getArguments(), codes, ARGLIST);
        codes->addCode(new CCALL(f, cond->getArguments()->size()));
        CBEZERO* cez = new CBEZERO;
        codes->addCode(cez);
        count += 2;
        int offset = transform(cond->getIfBlock(), codes, START);
        cez->setOffset(offset+1);
        count += offset;
      }
      break;
      default:
        //wxMessageBox("Unhandled AST-Type");
        break;
  }
  return count;
}

ExecutionContext::ExecutionContext(CodeSegment* segment) : mCode(segment), mStack(), mPC(0),
mHandler(NULL), mData(NULL), mSuspended(false){

}

ExecutionContext::~ExecutionContext(){
  delete mCode;
}

void PcdkScript::registerFunction(std::string name, ScriptFunc func){
  mFunctions[name] = func;
}

void PcdkScript::update(){
  for (std::list<ExecutionContext*>::iterator iter = mScripts.begin(); iter != mScripts.end(); ++iter){
    execute(*iter);
    if ((*iter)->mExecuteOnce){
      delete *iter;
      iter = mScripts.erase(iter);
    }
    if (iter == mScripts.end())
      break;
  }
}

void PcdkScript::execute(ExecutionContext* script, bool executeOnce){
  if (script == NULL)
    return;
  script->mExecuteOnce = executeOnce;
  mScripts.push_back(script);
}

void PcdkScript::execute(ExecutionContext* script){
  if (script->mSuspended)
    return;
  CCode* code = script->mCode->get(script->mPC);
  while(code){
    int result = script->mPC = code->execute(*script, script->mPC);
    if (script->mSuspended)
      break;
    code = script->mCode->get(script->mPC);
  }
  //script ran through
  if (!script->mSuspended && script->mPC >= script->mCode->numInstructions()){
    if (script->mHandler)
      script->mHandler(*script, script->mData);
    script->reset();
  }
}

void PcdkScript::remove(ExecutionContext* script){
  if (script == NULL)
    return;
  for (std::list<ExecutionContext*>::iterator iter = mScripts.begin(); iter != mScripts.end(); ++iter){
    if (*iter == script){
      iter = mScripts.erase(iter);
      break;
    }
    if (iter == mScripts.end())
      break;
  }
  script->reset();
  delete script;
}

int PcdkScript::loadRoom(ExecutionContext& ctx, unsigned numArgs){
  std::string room = ctx.stack().pop().getString();
  Engine::instance()->loadRoom(room);
  return 0;
}

int PcdkScript::setFocus(ExecutionContext& ctx, unsigned numArgs){
  std::string character = ctx.stack().pop().getString();
  Engine::instance()->setFocus(character);
  return 0;
}

int PcdkScript::showInfo(ExecutionContext& ctx, unsigned numArgs){
  std::string text = ctx.stack().pop().getString();
  bool show = ctx.stack().pop().getBool();
  if (show){
    Vec2i pos = Engine::instance()->getCursorPos();
    Vec2i ext = Engine::instance()->getFontRenderer()->getTextExtent(text, 1);
    Engine::instance()->getFontRenderer()->render(pos.x-ext.x/2, pos.y-ext.y, text, 1);
  }
  Engine::instance()->setObjectString(text);
  return 0;
}

int PcdkScript::walkTo(ExecutionContext& ctx, unsigned numArgs){
  std::string character = ctx.stack().pop().getString();
  Vec2i pos;
  pos.x = ctx.stack().pop().getInt()-1;
  pos.y = ctx.stack().pop().getInt()-1;
  pos = pos * Engine::instance()->getWalkGridSize();
  LookDir dir = UNSPECIFIED;
  if (numArgs >= 4)
    dir = (LookDir)(ctx.stack().pop().getInt()-1);
  bool hold = true;
  if (numArgs >= 5){
    std::string dw = ctx.stack().pop().getString();
    if (dw == "dontwait")
      hold = false;
  }
  CharacterObject* chr = Engine::instance()->getCharacter(character);
  if (chr){
    if (hold){
      chr->setSuspensionScript(&ctx);
      ctx.mSuspended = true;
    }
    Engine::instance()->walkTo(chr, pos, dir);
  }
  return 0;
}

int PcdkScript::speech(ExecutionContext& ctx, unsigned numArgs){
  std::string character = ctx.stack().pop().getString();
  std::string text = ctx.stack().pop().getString();
  std::string sound = "";
  if (numArgs >= 3) //TODO SOUND
    sound = ctx.stack().pop().getString();
  bool hold = true;
  if (numArgs >= 4){
    std::string dw = ctx.stack().pop().getString();
    if (dw == "dontwait")
      hold = false;
  }
  CharacterObject* chr = Engine::instance()->getCharacter(character);
  if (chr){
    Vec2i pos = chr->getOverheadPos();
    Vec2i ext = Engine::instance()->getFontRenderer()->getTextExtent(text, chr->getFontID());
    FontRenderer::String& str = Engine::instance()->getFontRenderer()->render(pos.x-ext.x/2,pos.y-ext.y, text, 
      chr->getFontID(), chr->getTextColor(), 3000);
    str.setSuspensionScript(&ctx);
    //stop speaking
    Engine::instance()->getFontRenderer()->removeText(chr);
    str.setSpeaker(chr);
    int currState = chr->getState();
    chr->setNextState(currState);
    chr->setState(CharacterObject::calculateState(currState, chr->isWalking(), true));
    if (sound != "") //TODO
      DebugBreak();
  }
  if (hold){
    ctx.mSuspended = true;
  }
  return 0;
}

int PcdkScript::pickup(ExecutionContext& ctx, unsigned numArgs){
  std::string character = ctx.stack().pop().getString();
  CharacterObject* chr = Engine::instance()->getCharacter(character);
  if (chr){
    chr->setNextState(chr->getState());
    LookDir dir = chr->getLookDir();
    if (dir == LEFT || dir == RIGHT)
      chr->setState(16);
    else if (dir == FRONT)
      chr->setState(15);
    chr->getAnimation()->registerAnimationEndHandler(chr);
  }
  return 0;
}

int PcdkScript::playSound(ExecutionContext& ctx, unsigned numArgs){
  std::string sound = ctx.stack().pop().getString();
  unsigned volume = 100;
  if (numArgs >= 2)
    volume = ctx.stack().pop().getInt();
  //TODO SOUND
  return 0;
}

int PcdkScript::setLight(ExecutionContext& ctx, unsigned numArgs){
  std::string room = ctx.stack().pop().getString();
  Color c;
  c.r = (unsigned char)ctx.stack().pop().getInt();
  c.g = (unsigned char)ctx.stack().pop().getInt();
  c.b = (unsigned char)ctx.stack().pop().getInt();
  bool fade = false;
  if (numArgs >= 5) //TODO
    fade = ctx.stack().pop().getString() == "fade";
  RoomObject* roomobj = Engine::instance()->getRoom(room);
  if (roomobj){
    roomobj->setLightingColor(c);
  }
  return 0;
}

int PcdkScript::setBool(ExecutionContext& ctx, unsigned numArgs){
  std::string boolname = ctx.stack().pop().getString();
  bool val = ctx.stack().pop().getBool();
  Engine::instance()->getInterpreter()->mBooleans[boolname] = val;
  return 0;
}

int PcdkScript::setObj(ExecutionContext& ctx, unsigned numArgs){
  std::string objname = ctx.stack().pop().getString();
  int state = ctx.stack().pop().getInt();
  for (unsigned i = 2; i < numArgs; ++i){
    DebugBreak(); //TODO state lists
    state = ctx.stack().pop().getInt();
  }
  Object2D* obj = Engine::instance()->getObject(objname);
  if (obj){
    obj->setState(state);
  }
  return 0;
}

int PcdkScript::beamTo(ExecutionContext& ctx, unsigned numArgs){
  return 0;
}

int PcdkScript::isBoolEqual(ExecutionContext& ctx, unsigned numArgs){
  std::string boolname = ctx.stack().pop().getString();
  bool test = ctx.stack().pop().getBool();
  bool saved = Engine::instance()->getInterpreter()->mBooleans[boolname];
  ctx.stack().push(StackData(test == saved));
  return 1;
}

int PcdkScript::isObjectInState(ExecutionContext& ctx, unsigned numArgs){
  std::string objname = ctx.stack().pop().getString();
  int checkstate = ctx.stack().pop().getInt();
  Object2D* obj = Engine::instance()->getObject(objname);
  if (obj){
    ctx.stack().push(StackData(checkstate == obj->getState()));
  }
  return 1;
}

int PcdkScript::isCommandSet(ExecutionContext& ctx, unsigned numArgs){
  EngineEvent check = EVT_NONE;
  if (numArgs >= 1){
    std::string evtname = ctx.stack().pop().getString();
    check = Engine::instance()->getInterpreter()->getEngineEvent(evtname);
  }
  EngineEvent evt = ctx.getCommandEvent();
  ctx.stack().push(StackData(check == evt));
  return 1;
}

EngineEvent PcdkScript::getEngineEvent(const std::string eventname){
  if (eventname == "mouse")
    return EVT_MOUSE;
  else if (eventname == "click")
    return EVT_CLICK;
  std::map<std::string,unsigned>::iterator iter = mData->getProjectSettings()->commands.find(eventname);
  if (iter != mData->getProjectSettings()->commands.end()){
    return static_cast<EngineEvent>(iter->second);
  }
  DebugBreak();
  return EVT_NONE;
}

void ExecutionContext::setEvent(EngineEvent evt){
  if (!mSuspended)
    mEvents.insert(evt);
}

void ExecutionContext::resetEvent(EngineEvent evt){
  mEvents.erase(evt);
}

bool ExecutionContext::isEventSet(EngineEvent evt){
  return mEvents.find(evt) != mEvents.end();
}

EngineEvent ExecutionContext::getCommandEvent(){
  for (std::set<EngineEvent>::iterator iter = mEvents.begin(); iter != mEvents.end(); ++iter){
    EngineEvent curr = *iter;
    if (curr >= EVT_USER_BEGIN && curr <= EVT_USER_MIRROR_END)
      return curr;
  }
  return EVT_NONE;
}

void ExecutionContext::reset(){
  mStack.clear();
  mEvents.clear();
  mPC = 0;
  mSuspended = false;
  delete mData;
  mData = NULL;
  mHandler = NULL;
}

void PcdkScript::clickEndHandler(ExecutionContext& ctx, void* data){
  Vec2i* pos = reinterpret_cast<Vec2i*>(data);
  if (ctx.isEventSet(EVT_CLICK)){
    ctx.resetEvent(EVT_CLICK);
    CharacterObject* chr = Engine::instance()->getCharacter("self");
    Engine::instance()->walkTo(chr, *((Vec2i*)data), UNSPECIFIED);
  }
  delete pos;
  ctx.mData = NULL;
}
