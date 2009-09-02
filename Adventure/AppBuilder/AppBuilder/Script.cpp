#include "Script.h"

#include <cassert>
#include <string>
#include <wx/msgdlg.h>

#include "Engine.h"

PcdkScript::PcdkScript(AdvDocument* data) : mData(data) {
  registerFunction("loadroom", loadRoom);
  registerFunction("setfocus", setFocus);
  registerFunction("showinfo", showInfo);
  registerFunction("walkto", walkTo);
  registerFunction("speech", setFocus);
  registerFunction("pickup", setFocus);
  registerFunction("playsound", setFocus);
}

PcdkScript::~PcdkScript(){

}

PcdkScript::ExecutionContext* PcdkScript::parseProgram(std::string program){
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

PcdkScript::ExecutionContext::ExecutionContext(CodeSegment* segment) : mCode(segment), mStack(), mPC(0){

}

PcdkScript::ExecutionContext::~ExecutionContext(){
  delete mCode;
}

void PcdkScript::registerFunction(std::string name, ScriptFunc func){
  mFunctions[name] = func;
}

void PcdkScript::execute(ExecutionContext* script){
  CCode* code = script->mCode->get(script->mPC);
  while(code){
    script->mPC = code->execute(script->mStack, script->mPC);
    code = script->mCode->get(script->mPC);
  }
  script->mPC = 0;
}

int PcdkScript::loadRoom(Stack& s, unsigned numArgs){
  std::string room = s.pop().getString();
  Engine::instance()->loadRoom(room);
  return 0;
}

int PcdkScript::setFocus(Stack& s, unsigned numArgs){
  std::string character = s.pop().getString();
  Engine::instance()->setFocus(character);
  return 0;
}

int PcdkScript::showInfo(Stack& s, unsigned numArgs){
  std::string text = s.pop().getString();
  bool show = s.pop().getBool();
  if (show){
    Vec2i pos = Engine::instance()->getCursorPos();
    Vec2i ext = Engine::instance()->getFontRenderer()->getTextExtent(text, 1);
    Engine::instance()->getFontRenderer()->render(pos.x-ext.x/2, pos.y-ext.y, text, 1);
  }
  Engine::instance()->setObjectString(text);
  return 0;
}

int PcdkScript::walkTo(Stack& s, unsigned numArgs){
  std::string character = s.pop().getString();
  Vec2i pos;
  pos.x = s.pop().getInt()-1;
  pos.y = s.pop().getInt()-1;
  pos = pos * Engine::instance()->getWalkGridSize();
  LookDir dir = (LookDir)(s.pop().getInt()-1);
  CharacterObject* chr = Engine::instance()->getCharacter(character);
  if (chr){
    Engine::instance()->walkTo(chr, pos, dir);
  }
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

void PcdkScript::setEvent(EngineEvent evt){
  mEvents.insert(evt);
}

void PcdkScript::resetEvent(EngineEvent evt){
  mEvents.erase(evt);
}

bool PcdkScript::isEventSet(EngineEvent evt){
  return mEvents.find(evt) != mEvents.end();
}
