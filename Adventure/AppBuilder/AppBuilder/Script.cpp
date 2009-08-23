#include "Script.h"

#include <cassert>
#include <string>

#include "Engine.h"

PcdkScript::PcdkScript(){
  registerFunction("loadroom", loadRoom);
  registerFunction("setfocus", setFocus);
}

PcdkScript::~PcdkScript(){

}

PcdkScript::CodeSegment* PcdkScript::parseProgram(std::string program){
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
    //todo
  }
  parser->free(parser);
  tokStream->free(tokStream);
  lexer->free(lexer);
  input->free(input);
  CodeSegment* segment = new CodeSegment;
  transform(p, segment, START);
  delete p;
  return segment;
}

void PcdkScript::transform(NodeList* program, CodeSegment* codes, TrMode mode){
  if (mode == ARGLIST){
    program->reset(false);
    while(program->hasPrev()){
      ASTNode* ast = program->prev();
      transform(ast, codes);
    }
  }
  else{
    program->reset(true);
    while(program->hasNext()){
      ASTNode* ast = program->next();
      transform(ast, codes);
    }
  }
}

void PcdkScript::transform(ASTNode* node, CodeSegment* codes){
  switch(node->getType()){
      case ASTNode::FUNCCALL:{
        FuncCallNode* fc = static_cast<FuncCallNode*>(node);
        ScriptFunc f = mFunctions[fc->getName()];
        assert(f != NULL);
        transform(fc->getArguments(), codes, ARGLIST);
        codes->addCode(new CCALL(f, fc->getArguments()->size()));
      }
      break;
      case ASTNode::IDENTIFIER:{
        IdentNode* id = static_cast<IdentNode*>(node);
        codes->addCode(new CPUSH(id->value()));
      }
      break;
  }
}

void PcdkScript::registerFunction(std::string name, ScriptFunc func){
  mFunctions[name] = func;
}

void PcdkScript::execute(CodeSegment* segment){
  mPc = 0;
  CCode* code = segment->get(mPc);
  while(code){
    mPc = code->execute(mStack, mPc);
    code = segment->get(mPc);
  }
}

int PcdkScript::loadRoom(Stack& s, unsigned numArgs){
  std::string room = s.pop().getString();
  Engine::instance()->loadRoom(room);
  return 0;
}

int PcdkScript::setFocus(Stack& s, unsigned numArgs){
  std::string character = s.pop().getString();
  return 0;
}
