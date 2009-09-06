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
        ScriptFunc f = mFunctions[fc->getName()];
        assert(f != NULL);
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
    Engine::instance()->walkTo(chr, pos, dir);
  }
  if (hold){
    chr->setSuspensionScript(&ctx);
    ctx.mSuspended = true;
  }
  return 0;
}

int PcdkScript::speech(ExecutionContext& ctx, unsigned numArgs){
  std::string character = ctx.stack().pop().getString();
  std::string text = ctx.stack().pop().getString();
  std::string sound = "";
  if (numArgs >= 3)
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
    str.setSpeaker(chr);
    int currState = chr->getState();
    chr->setNextState(currState);
    chr->setState(CharacterObject::calculateState(currState, chr->isWalking(), true));
  }
  if (hold){
    ctx.mSuspended = true;
  }
  return 0;
}

int PcdkScript::pickup(ExecutionContext& ctx, unsigned numArgs){
  return 0;
}

int PcdkScript::playSound(ExecutionContext& ctx, unsigned numArgs){
  return 0;
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
  assert(false);
  return EVT_UNKNOWN;
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
